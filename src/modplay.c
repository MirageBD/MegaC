#include <calypsi/intrinsics6502.h>

#include <stdint.h>
#include "macros.h"
#include "registers.h"
#include "dma.h"

/*
	for an 125 BPM song (125 being the highest), _my_ tempo is $0ea0 raster lines, which is (3744/312) = 12 screens (but for some reason 6... physical vs other raster lines???)
	but the ticks per row can be anything from 1 to 31 (6 being the default)
*/

/*
	For amiga:

	CPU clock =                  7093.7892 kHz (7 Mhz)
	CIA clock = CPU clock / 10 = 709.37892 kHz

	50Hz timer = CPU Clock / 500 = 14187.5784
	tempo num = 50Hz timer * 125bpm = 1773447

	because the timer is only a word ($ffff) the minumum value is 28 (1773447/27 = $10093 is too big, 1773447/28 = $0f769, which fits).

	tempo calculation goes like this for a 50Hz PAL machine with 312 raster lines

	number of rasters * frames in second * 60 seconds
	RASTERS_PER_MINUTE = 312 rasters per frame * 50 frames per second * 60 seconds = 936000 raster lines per minute

	tempo = (RASTERS_PER_MINUTE / BEATS_PER_MINUTE) / 2;
	      = (936000 / 125) / 2 = 3744 = $0EA0
*/

/*
	Protracker CIA (Complex Interface Adapter) Timer Tempo Calculations:
	--------------------------------------------------------------------
	Fcolor                        = 4.43361825 MHz (PAL color carrier frequency)
	CPU Clock   = Fcolor * 1.6    = 7.0937892  MHz
	CIA Clock   = Cpu Clock / 10  = 709.37892  kHz

	50 Hz Timer = CIA Clock / 50  = 14187.5784
	Tempo num.  = 50 Hz Timer*125 = 1773447

	For NTSC: CPU Clock = 7.1590905 MHz --> Tempo num. = 1789773

	To calculate tempo we use the formula: TimerValue = 1773447 / Tempo
	The timer is only a word, so the available tempo range is 28-255 (++).
	Tempo 125 will give a normal 50 Hz timer (VBlank).

	A normal Protracker VBlank song tempo can be calculated as follows:
	We want to know the tempo in BPM (Beats Per Minute), or rather quarter-
	notes per minute. Four notes makes up a quarternote.
	First find interrupts per minute: 60 seconds * 50 per second = 3000
	Divide by interrupts per quarter note = 4 notes * speed
	This gives: Tempo = 3000/(4*speed)
	simplified: Tempo = 750/speed
	For a normal song in speed 6 this formula gives: 750/6 = 125 BPM
*/

/*
	AXELF.MOD        0x0e94 rasterline wait from trying randomly
	tempo = 125
	ticks/row = 6	IS THIS 'SPEED'?
	rows/beat = 4
*/

#define MAX_INSTRUMENTS						32
#define CPU_FREQUENCY						40500000
#define AMIGA_PAULA_CLOCK					(70937892 / 20) // CPU clock / 20
#define RASTERS_PER_SECOND					(unsigned long)(312 * 50)	// PAL 0-311, NTSC 0-262
#define RASTERS_PER_MINUTE					(unsigned long)(RASTERS_PER_SECOND * 60)
#define ROWS_PER_BEAT						4

#define NUM_SIGS							4
char mod31_sigs[NUM_SIGS][4] =
{
	{ 0x4D, 0x2e, 0x4b, 0x2e },	// M.K.
	{ 0x4D, 0x21, 0x4b, 0x21 }, // M!K!
	{ 0x46, 0x4c, 0x54, 0x34 }, // FLT4
	{ 0x46, 0x4c, 0x54, 0x38 }  // FLT8
	// 'M.K.','8CHN', '4CHN','6CHN','FLT4','FLT8' = 31 instruments.
};

unsigned long	load_addr;
unsigned char	mod_tmpbuf[23];

/*
  Period = 214 = 16574.27 Hz sample rate. Time base gets added 40.5M times per second.
  16574.27 * 2^24 / 40.5M = 6866 for period = 214
  If period is higher, then this number will be lower, so it should be 6866*214 / PERIOD = 1469038 / PERIOD

  If we calculate N = 2^24 / PERIOD, then what we want will be (1469038 x N ) >> 24
  But our multiplier is only 23x18 bits so N = 2^16 / PERIOD should fit, and still allow us to just shift the result right by 2 bytes
*/

long			sample_rate_divisor			= 1469038;

unsigned char   beatsperminute				= 125;
unsigned short	song_offset					= 0;
unsigned short	tempo						= 0;
unsigned long	sample_data_start			= 0x40000;
unsigned char	max_pattern					= 0;
unsigned char	current_pattern_index		= 0;
unsigned char	current_pattern				= 0;
unsigned char	current_pattern_position	= 0;
unsigned char	numpatternindices;
unsigned char	song_loop_point; // unused
unsigned short	top_addr;
unsigned short	freq;

unsigned short	instrument_lengths			[MAX_INSTRUMENTS];
unsigned short	instrument_loopstart		[MAX_INSTRUMENTS];
unsigned short	instrument_looplen			[MAX_INSTRUMENTS];
unsigned char	instrument_finetune			[MAX_INSTRUMENTS];
unsigned long	instrument_addr				[MAX_INSTRUMENTS];
unsigned char	instrument_vol				[MAX_INSTRUMENTS];
unsigned char	song_pattern_list			[128];
unsigned char	last_instruments			[4] = { 0, 0, 0, 0 };
unsigned char	pattern_buffer				[16];

unsigned char  instrument;
unsigned short period;
unsigned short effect;

// ------------------------------------------------------------------------------------

void modplay_playnote(unsigned char channel, unsigned char *note)
{
	//	if (channel > 3)
	//		return;

	instrument  = note[0]  & 0xf0;
	instrument |= note[2] >> 4;

	if(!instrument)
		instrument = last_instruments[channel];
	else
		instrument--;

	last_instruments[channel] = instrument;

	period = ((note[0] & 0xf) << 8) + note[1];
	effect = ((note[2] & 0xf) << 8) + note[3];

	unsigned ch_ofs = channel << 4;

	if(period)
	{
		freq = 0xFFFFL / period; // result gets shifted by 2 bytes, see explanation at top

		// Stop playback while loading new sample data
		poke(0xd720 + ch_ofs, 0x00);

		// Load sample address into base and current addr
		poke(0xd721 + ch_ofs, (((unsigned short)instrument_addr[instrument]) >> 0 ) & 0xff);
		poke(0xd722 + ch_ofs, (((unsigned short)instrument_addr[instrument]) >> 8 ) & 0xff);
		poke(0xd723 + ch_ofs, (((unsigned long )instrument_addr[instrument]) >> 16) & 0xff);
		poke(0xd72a + ch_ofs, (((unsigned short)instrument_addr[instrument]) >> 0 ) & 0xff);
		poke(0xd72b + ch_ofs, (((unsigned short)instrument_addr[instrument]) >> 8 ) & 0xff);
		poke(0xd72c + ch_ofs, (((unsigned long )instrument_addr[instrument]) >> 16) & 0xff);

		// Sample top address
		top_addr = instrument_addr[instrument] + instrument_lengths[instrument];
		poke(0xd727 + ch_ofs, (top_addr >> 0) & 0xff);
		poke(0xd728 + ch_ofs, (top_addr >> 8) & 0xff);

		// Volume
		poke(0xd729 + ch_ofs, instrument_vol[instrument] >> 2);

		// Mirror channel quietly on other side for nicer stereo imaging
		poke(0xd71c + channel, instrument_vol[instrument] >> 4);

		// XXX - We should set base addr and top addr to the looping range, if the sample has one.
		if (instrument_loopstart[instrument])
		{
			// start of loop
			poke(0xd721 + ch_ofs, (((unsigned long )instrument_addr[instrument] + 2 * instrument_loopstart[instrument]                                       ) >> 0 ) & 0xff);
			poke(0xd722 + ch_ofs, (((unsigned long )instrument_addr[instrument] + 2 * instrument_loopstart[instrument]                                       ) >> 8 ) & 0xff);
			poke(0xd723 + ch_ofs, (((unsigned long )instrument_addr[instrument] + 2 * instrument_loopstart[instrument]                                       ) >> 16) & 0xff);

			// Top addr
			poke(0xd727 + ch_ofs, (((unsigned short)instrument_addr[instrument] + 2 * (instrument_loopstart[instrument] + instrument_looplen[instrument] - 1)) >> 0 ) & 0xff);
			poke(0xd728 + ch_ofs, (((unsigned short)instrument_addr[instrument] + 2 * (instrument_loopstart[instrument] + instrument_looplen[instrument] - 1)) >> 8 ) & 0xff);
		}

		poke(0xd770, sample_rate_divisor      );
		poke(0xd771, sample_rate_divisor >> 8 );
		poke(0xd772, sample_rate_divisor >> 16);
		poke(0xd773, 0x00                     );

		poke(0xd774, freq     );
		poke(0xd775, freq >> 8);
		poke(0xd776, 0        );

		// Pick results from output / 2^16
		poke(0xd724 + ch_ofs, peek(0xd77a));
		poke(0xd725 + ch_ofs, peek(0xd77b));
		poke(0xd726 + ch_ofs, 0);

		if (instrument_loopstart[instrument])
		{
			// Enable playback+ nolooping of channel 0, 8-bit, no unsigned samples
			poke(0xd720 + ch_ofs, 0xC2);
		}
		else
		{
			// Enable playback+ nolooping of channel 0, 8-bit, no unsigned samples
			poke(0xd720 + ch_ofs, 0x82);
		}
	}

	switch (effect & 0xf00)
	{
		case 0xf00: // Speed!!!
			if ((effect & 0x0ff) < 0x20) // speed (00-1F) / tempo (20-FF)
			{
				tempo = RASTERS_PER_MINUTE / beatsperminute / 2; // ROWS_PER_BEAT;
				tempo *= 6 / (effect & 0x1f); // F - Set Speed Fxx : speed (00-1F) / tempo (20-FF)
			}
			else // effect & 0x0ff >= 0x20
			{
				beatsperminute = (effect & 0xff);
				tempo = RASTERS_PER_MINUTE / beatsperminute / 2; // / ROWS_PER_BEAT;
			}
			break;
		case 0xc00: // Channel volume
			poke(0xd729 + ch_ofs, effect & 0xff);
			break;
	}

	// Enable audio dma, enable bypass of audio mixer, signed samples
	poke(0xd711, 0x80);
}

void modplay_play()
{
	// play pattern row
	dma_lcopy(load_addr + song_offset + (current_pattern << 10) + (current_pattern_position << 4), pattern_buffer, 16);

	modplay_playnote(0, &pattern_buffer[0 ]);
	modplay_playnote(1, &pattern_buffer[4 ]);
	modplay_playnote(2, &pattern_buffer[8 ]);
	modplay_playnote(3, &pattern_buffer[12]);

	current_pattern_position++;
	
	if(current_pattern_position > 0x3f)
	{
		current_pattern_position = 0x00;
		current_pattern_index++;
		if(current_pattern_index == numpatternindices)
			current_pattern_index = 0;
		current_pattern = song_pattern_list[current_pattern_index];
		current_pattern_position = 0;
	}
}

void modplay_init(unsigned long address)
{
	unsigned short i;
	unsigned char a;
	unsigned char numinstruments;

	load_addr = address;

	dma_lcopy(load_addr + 1080, mod_tmpbuf, 4);								// Check if 15 or 31 instrument mode (M.K.)

	mod_tmpbuf[4] = 0;
	numinstruments = 15;
	song_offset = 1080 - (16 * 30);

	for(i = 0; i < NUM_SIGS; i++)
	{
		for(a = 0; a < 4; a++)
			if(mod_tmpbuf[a] != mod31_sigs[i][a])
				break;
		if(a == 4)
		{
			numinstruments = 31;
			song_offset = 1084;												// case for 31 instruments
		}
	}

	for(i = 0; i < numinstruments; i++)
	{
		// 2 bytes - sample length in words. multiply by 2 for byte length
		// 1 byte  - lower fout bits for finetune, upper for
		// 1 byte  - volume for sample ($00-$40)
		// 2 bytes - repeat point in words
		// 2 bytes - repeat length in words
		dma_lcopy(load_addr + 0x14 + i * 30 + 22, mod_tmpbuf, 8);			// Get instrument data for plucking
		instrument_lengths   [i] = mod_tmpbuf[1] + (mod_tmpbuf[0] << 8);
		instrument_lengths   [i] <<= 1;										// Redenominate instrument length into bytes
		instrument_finetune  [i] = mod_tmpbuf[2];
		instrument_vol       [i] = mod_tmpbuf[3];							// Instrument volume
		instrument_loopstart [i] = mod_tmpbuf[5] + (mod_tmpbuf[4] << 8);	// Repeat start point and end point
		instrument_looplen   [i] = mod_tmpbuf[7] + (mod_tmpbuf[6] << 8);
	}

	numpatternindices = lpeek(load_addr + 20 + numinstruments*30 + 0);
	song_loop_point = lpeek(load_addr + 20 + numinstruments*30 + 1);

	dma_lcopy(load_addr + 20 + numinstruments*30 + 2, song_pattern_list, 128);
	for(i = 0; i < numpatternindices; i++)
	{
		if(song_pattern_list[i] > max_pattern)
			max_pattern = song_pattern_list[i];
	}

	sample_data_start = load_addr + song_offset + ((unsigned long)max_pattern + 1) * 1024;

	for(i = 0; i < MAX_INSTRUMENTS; i++)
	{
		instrument_addr[i] = sample_data_start;
		sample_data_start += instrument_lengths[i];
	}

	current_pattern_index = 0;
	current_pattern = song_pattern_list[0];
	current_pattern_position = 0;

	unsigned long ticksperrow = 6;
	unsigned long bpm = 125;
	tempo = RASTERS_PER_MINUTE / beatsperminute;
	tempo = tempo / 2;

	// while(1) { poke(0xd020, peek(0xd020)+1); }

	// audioxbar_setcoefficient(i, 0xff);
	for(i = 0; i < 256; i++)
	{
		// Select the coefficient
		poke(0xd6f4, i);

		// Now wait at least 16 cycles for it to settle
		poke(0xd020U, peek(0xd020U));
		poke(0xd020U, peek(0xd020U));

		// set value to 0xff
		poke(0xd6f5U, 0xff);
	}
}
