#include <calypsi/intrinsics6502.h>

#include <stdint.h>
#include "macros.h"
#include "registers.h"
// #include <memory.h>

// lcopy implementation, move somewhere else! ------------------------------------------

/*
.macro DMA_RUN_JOB jobPointer
		lda #(jobPointer & $ff0000) >> 16
		sta $d702
		sta $d704
		lda #>jobPointer
		sta $d701
		lda #<jobPointer
		sta $d705
.endmacro
*/


struct dmagic_dmalist
{
	unsigned char command;
	unsigned int  count;
	unsigned int  source_addr;
	unsigned char source_bank;
	unsigned int  dest_addr;
	unsigned char dest_bank;
	unsigned int  modulo;
};

struct dmagic_dmalist dmalist;

void do_dma(void)
{
	// Gate C65 IO enable
	// poke(0xd02fU,0xa5);
	// poke(0xd02fU,0x96);
	// Force back to 3.5MHz
	// POKE(0xD031,PEEK(0xD031)|0x40);

	poke(0xd702U, 0);
	poke(0xd704U, 0);
	// everything OK here
	//poke(0xd705U, 0);
	// everything not OK here
	//poke(0xd706U, 0);

	poke(0xd701U,((unsigned int)&dmalist)>>8);
	poke(0xd700U,((unsigned int)&dmalist)&0xff); // triggers DMA
}

void lcopy(long source_address, long destination_address, unsigned int count)
{
	dmalist.command     = 0x00; // copy
	dmalist.count       = count;
	dmalist.source_addr = source_address & 0xffff;
	dmalist.source_bank = (source_address >> 16) & 0x7f;
	dmalist.dest_addr   = destination_address & 0xffff;
	dmalist.dest_bank   = (destination_address >> 16) & 0x7f;

	do_dma();

	return;
}

// end of lcopy implementation ------------------------------------------------------------------------------------

#define MAX_INSTRUMENTS						32
#define CPU_FREQUENCY						40500000
#define AMIGA_PAULA_CLOCK					(70937892 / 20)
#define RASTERS_PER_SECOND					(313 * 50)
#define RASTERS_PER_MINUTE					(RASTERS_PER_SECOND * 60)
#define BEATS_PER_MINUTE					125
#define ROWS_PER_BEAT						8

#define NUM_SIGS							4
char mod31_sigs[NUM_SIGS][4] =
{
	{ 0x4D, 0x2e, 0x4b, 0x2e },
	{ 0x4D, 0x21, 0x4b, 0x21 },
	{ 0x46, 0x4c, 0x54, 0x34 },
	{ 0x46, 0x4c, 0x54, 0x38 }
};

char			msg[64 + 1];
char			peak_msg[40 + 1];

unsigned short	i;
unsigned char	a, b, c, d;
unsigned char	fd;

int				count;
unsigned char	buffer[512];

unsigned long	load_addr;

unsigned char	mod_name[23];

long			sample_rate_divisor			= 1469038;
unsigned short	song_offset					= 1084;
unsigned short	tempo						= RASTERS_PER_MINUTE / BEATS_PER_MINUTE / ROWS_PER_BEAT;
unsigned char	histo_bins[640];
unsigned char	random_target				= 40;
unsigned char	last_random_target			= 40;
unsigned int	random_seek_count			= 0;
unsigned char	request_track				= 40;
unsigned char	read_sectors[41]			= { 0 };
unsigned char	last_track_seen				= 255;
unsigned int	histo_samples				= 0;

unsigned short	instrument_lengths			[MAX_INSTRUMENTS];
unsigned short	instrument_loopstart		[MAX_INSTRUMENTS];
unsigned short	instrument_looplen			[MAX_INSTRUMENTS];
unsigned char	instrument_finetune			[MAX_INSTRUMENTS];
unsigned long	instrument_addr				[MAX_INSTRUMENTS];
unsigned char	instrument_vol				[MAX_INSTRUMENTS];

unsigned char	song_length;
unsigned char	song_loop_point;
unsigned char	song_pattern_list[128];
unsigned char	max_pattern					= 0;
unsigned long	sample_data_start			= 0x40000;

unsigned long	time_base					= 0;

unsigned char	current_pattern_in_song		= 0;
unsigned char	current_pattern				= 0;
unsigned char	current_pattern_position	= 0;

unsigned char	screen_first_row			= 0;

unsigned		ch_en[4]					= { 1, 1, 1, 1 };
unsigned char	last_instruments[4]			= { 0, 0, 0, 0 };

unsigned char	pattern_buffer[16];
char			note_fmt[9 + 1];

unsigned short	top_addr;
unsigned short	freq;

// ------------------------------------------------------------------------------------

void audioxbar_setcoefficient(uint8_t n, uint8_t value)
{
	// Select the coefficient
	poke(0xd6f4, n);

	// Now wait at least 16 cycles for it to settle
	poke(0xd020U, peek(0xd020U));
	poke(0xd020U, peek(0xd020U));

	poke(0xd6f5U, value);
}

void play_sample(unsigned char channel, unsigned char instrument, unsigned short period, unsigned short effect)
{
	unsigned ch_ofs = channel << 4;

	if (channel > 3)
		return;

	freq = 0xFFFFL / period;

	// Stop playback while loading new sample data
	poke(0xD720 + ch_ofs, 0x00);

	// Load sample address into base and current addr
	poke(0xD721 + ch_ofs, (((unsigned short)instrument_addr[instrument]) >> 0 ) & 0xff);
	poke(0xD722 + ch_ofs, (((unsigned short)instrument_addr[instrument]) >> 8 ) & 0xff);
	poke(0xD723 + ch_ofs, (((unsigned long )instrument_addr[instrument]) >> 16) & 0xff);
	poke(0xD72A + ch_ofs, (((unsigned short)instrument_addr[instrument]) >> 0 ) & 0xff);
	poke(0xD72B + ch_ofs, (((unsigned short)instrument_addr[instrument]) >> 8 ) & 0xff);
	poke(0xD72C + ch_ofs, (((unsigned long )instrument_addr[instrument]) >> 16) & 0xff);

	// Sample top address
	top_addr = instrument_addr[instrument] + instrument_lengths[instrument];
	poke(0xD727 + ch_ofs, (top_addr >> 0) & 0xff);
	poke(0xD728 + ch_ofs, (top_addr >> 8) & 0xff);

	// Volume
	poke(0xD729 + ch_ofs, instrument_vol[instrument] >> 2);

	// Mirror channel quietly on other side for nicer stereo imaging
	poke(0xD71C + channel, instrument_vol[instrument] >> 4);

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

	switch (effect & 0xf00)
	{
		case 0xf00: // Tempo/Speed
			if ((effect & 0x0ff) < 0x20)
			{
				tempo = RASTERS_PER_MINUTE / BEATS_PER_MINUTE / ROWS_PER_BEAT;
				tempo = tempo * 6 / (effect & 0x1f);
			}
			break;
		case 0xc00: // Channel volume
			poke(0xd729 + ch_ofs, effect & 0xff);
			break;
	}

	// Enable audio dma, enable bypass of audio mixer, signed samples
	poke(0xd711, 0x80);
}

void play_note(unsigned char channel, unsigned char *note)
{
	unsigned char instrument;
	unsigned short period;
	unsigned short effect;

	instrument = note[0] & 0xf0;
	instrument |= note[2] >> 4;

	if (!instrument)
		instrument = last_instruments[channel];
	else
		instrument--;

	last_instruments[channel] = instrument;

	period = ((note[0] & 0xf) << 8) + note[1];
	effect = ((note[2] & 0xf) << 8) + note[3];

	if (period)
		play_sample(channel, instrument, period, effect);
}

void play_mod_pattern_line(void)
{
	// Get pattern row
	lcopy(0x40000 + song_offset + (current_pattern << 10) + (current_pattern_position << 4), pattern_buffer, 16);
	if (ch_en[0]) play_note(0, &pattern_buffer[0 ]);
	if (ch_en[1]) play_note(1, &pattern_buffer[4 ]);
	if (ch_en[2]) play_note(2, &pattern_buffer[8 ]);
	if (ch_en[3]) play_note(3, &pattern_buffer[12]);
}

void load_modfile(void)
{
	song_offset = 1084;
	load_addr = 0x40000;

	// Check if 15 or 31 instrument mode
	lcopy(0x40000 + 1080, mod_name, 4);									// M.K.

	mod_name[4] = 0;
	song_offset = 1084 - (16 * 30);

	for (i = 0; i < NUM_SIGS; i++)
	{
		for (a = 0; a < 4; a++)
			if (mod_name[a] != mod31_sigs[i][a])
				break;
		if (a == 4)
			song_offset = 1084;
	}

	for (i = 0; i < (song_offset == 1084 ? 31 : 15); i++)
	{
		
		lcopy(0x40014 + i * 30 + 22, mod_name, 22);						// Get instrument data for plucking
		instrument_lengths[i] = mod_name[1] + (mod_name[0] << 8);

		instrument_lengths[i] <<= 1;									// Redenominate instrument length into bytes
		instrument_finetune[i] = mod_name[2];

		instrument_vol[i] = mod_name[3];								// Instrument volume

		instrument_loopstart[i] = mod_name[5] + (mod_name[4] << 8);		// Repeat start point and end point
		instrument_looplen[i] = mod_name[7] + (mod_name[6] << 8);
	}

	song_length = lpeek(0x40000 + 950);
	song_loop_point = lpeek(0x40000 + 951);

	lcopy(0x40000 + 952, song_pattern_list, 128);
	for (i = 0; i < song_length; i++)
	{
		if (song_pattern_list[i] > max_pattern)
			max_pattern = song_pattern_list[i];
	}

	sample_data_start = 0x40000L + song_offset + (max_pattern + 1) * 1024;

	for (i = 0; i < MAX_INSTRUMENTS; i++)
	{
		instrument_addr[i] = sample_data_start;
		sample_data_start += instrument_lengths[i];
	}

	current_pattern_in_song = 0;
	current_pattern = song_pattern_list[0];
	current_pattern_position = 0;
}

/*
void main(void)
{
	unsigned char ch;
	unsigned char playing = 0;

	// Fast CPU, M65 IO
	poke(0, 65);
	poke(0xd02F, 0x47);
	poke(0xd02F, 0x53);

	while (peek(0xd610))
		poke(0xd610, 0);

	// Stop all DMA audio first
	poke(0xd720, 0);
	poke(0xd730, 0);
	poke(0xd740, 0);
	poke(0xd750, 0);

	// Audio cross-bar full volume
	for (i = 0; i < 256; i++)
		audioxbar_setcoefficient(i, 0xff);

	load_modfile();

	while(1)
	{
		play_mod_pattern_line();

		// wait 'tempo' rasterlines
		for (i = 0; i < tempo; i++)
		{
			c = peek(0xd012);
			while (peek(0xd012) == c)
				continue;
		}
		
		current_pattern_position++;
		
		if (current_pattern_position > 0x3f)
		{
			current_pattern_position = 0x00;
			current_pattern_in_song++;
			if (current_pattern_in_song == song_length)
				current_pattern_in_song = 0;
			current_pattern = song_pattern_list[current_pattern_in_song];
			current_pattern_position = 0;
		}
	}

	// base addr = $040000
	poke(0xd721, 0x00);
	poke(0xd722, 0x00);
	poke(0xd723, 0x04);

	// time base = $001000
	poke(0xd724, 0x01);
	poke(0xd725, 0x10);
	poke(0xd726, 0x00);

	// Top address
	poke(0xd727, 0xFE);
	poke(0xd728, 0xFF);

	// Full volume
	poke(0xd729, 0xFF);

	// Enable audio dma, 16 bit samples
	poke(0xd711, 0x80);
}
*/
