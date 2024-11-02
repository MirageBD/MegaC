#include <calypsi/intrinsics6502.h>
#include <stdint.h>
#include "macros.h"
#include "registers.h"
#include "dma.h"

#define MAX_INSTRUMENTS						32
#define NUMRASTERS							(unsigned long)(2 * 312)
#define RASTERS_PER_SECOND					(unsigned long)(NUMRASTERS * 50)	// PAL 0-311, NTSC 0-262
#define RASTERS_PER_MINUTE					(unsigned long)(RASTERS_PER_SECOND * 60)
#define ROWS_PER_BEAT						4

#define NUM_SIGS							4
char mod31_sigs[NUM_SIGS][4] =
{
	{ 0x4D, 0x2e, 0x4b, 0x2e },	// M.K.
	{ 0x4D, 0x21, 0x4b, 0x21 }, // M!K!
	{ 0x46, 0x4c, 0x54, 0x34 }, // FLT4
	{ 0x46, 0x4c, 0x54, 0x38 }  // FLT8
};

unsigned long	load_addr;
unsigned char	mod_tmpbuf[23];

long			sample_rate_divisor			= 1468299;

unsigned char   beats_per_minute			= 125;
unsigned short	song_offset					= 0;
unsigned long	sample_data_start			= 0;
unsigned char	max_pattern					= 0;
unsigned char	current_pattern_index		= 0;
unsigned char	current_pattern				= 0;
unsigned char	current_row					= 0;
unsigned char	numpatternindices;
unsigned char	song_loop_point; // unused
unsigned short	top_addr;

unsigned short	sample_lengths				[MAX_INSTRUMENTS];
unsigned short	sample_loopstart			[MAX_INSTRUMENTS];
unsigned short	sample_looplen				[MAX_INSTRUMENTS];
unsigned char	sample_finetune				[MAX_INSTRUMENTS];
unsigned long	sample_addr					[MAX_INSTRUMENTS];
unsigned char	sample_vol					[MAX_INSTRUMENTS];
unsigned char	song_pattern_list			[128];
unsigned char	pattern_buffer				[16];

unsigned char   samplenum;
unsigned char   effectlo;
unsigned char   effecthi;
unsigned char   periodlo;
unsigned char   periodhi;
unsigned char   freqlo;
unsigned char   freqhi;

unsigned char*  sample_address_ptr;
unsigned char   sample_address0;
unsigned char   sample_address1;
unsigned char   sample_address2;
unsigned char   sample_address3;

extern unsigned short	tempo;
extern unsigned char	ticks;
extern unsigned char	speed;

// ------------------------------------------------------------------------------------

void modplay_playnote_c(unsigned char channel, unsigned char *note)
{
	samplenum  = note[0] & 0xf0;
	samplenum |= note[2] >> 4;
	samplenum--;

	periodlo = note[1];
	periodhi = (note[0] & 0xf);

	effectlo = note[3];
	effecthi = (note[2] & 0xf);

	unsigned char ch_ofs = channel << 4;

	if((periodlo | periodhi) != 0)
	{
		poke(0xd720 + ch_ofs, 0x00);																						// Stop playback while loading new sample data

		sample_address_ptr = (unsigned char*)sample_addr + samplenum * 4;
		sample_address0 = *(sample_address_ptr+0);
		sample_address1 = *(sample_address_ptr+1);
		sample_address2 = *(sample_address_ptr+2);
		sample_address3 = *(sample_address_ptr+3);

		poke(0xd721 + ch_ofs, sample_address0);																				// Load sample address into base and current addr
		poke(0xd722 + ch_ofs, sample_address1);
		poke(0xd723 + ch_ofs, sample_address2);
		poke(0xd72a + ch_ofs, sample_address0);
		poke(0xd72b + ch_ofs, sample_address1);
		poke(0xd72c + ch_ofs, sample_address2);

		top_addr = sample_addr[samplenum] + sample_lengths[samplenum];														// Sample top address
		poke(0xd727 + ch_ofs, (top_addr >> 0) & 0xff);
		poke(0xd728 + ch_ofs, (top_addr >> 8) & 0xff);

		poke(0xd729 + ch_ofs, sample_vol[samplenum]);																		// Volume
		poke(0xd71c + channel, sample_vol[samplenum]);																		// Mirror channel quietly on other side for nicer stereo imaging

		if (sample_loopstart[samplenum])																					// XXX - We should set base addr and top addr to the looping range, if the sample has one.
		{
			// start of loop
			poke(0xd721 + ch_ofs, (((unsigned long )sample_addr[samplenum] + 2 * sample_loopstart[samplenum]                                  ) >> 0 ) & 0xff);
			poke(0xd722 + ch_ofs, (((unsigned long )sample_addr[samplenum] + 2 * sample_loopstart[samplenum]                                  ) >> 8 ) & 0xff);
			poke(0xd723 + ch_ofs, (((unsigned long )sample_addr[samplenum] + 2 * sample_loopstart[samplenum]                                  ) >> 16) & 0xff);

			// Top addr
			poke(0xd727 + ch_ofs, (((unsigned short)sample_addr[samplenum] + 2 * (sample_loopstart[samplenum] + sample_looplen[samplenum] - 1)) >> 0 ) & 0xff);
			poke(0xd728 + ch_ofs, (((unsigned short)sample_addr[samplenum] + 2 * (sample_loopstart[samplenum] + sample_looplen[samplenum] - 1)) >> 8 ) & 0xff);
		}

		MATH.MULTINA0 = 0xff;																								// calculate frequency // freq = 0xFFFFL / period
		MATH.MULTINA1 = 0xff;
		MATH.MULTINA2 = 0;
		MATH.MULTINA3 = 0;

		MATH.MULTINB0 = periodlo;
		MATH.MULTINB1 = periodhi;
		MATH.MULTINB2 = 0;
		MATH.MULTINB3 = 0;

		__asm(
			" lda 0xd020\n"
			" sta 0xd020\n"
			" lda 0xd020\n"
			" sta 0xd020\n"
			" lda 0xd020\n"
			" sta 0xd020\n"
			" lda 0xd020\n"
			" sta 0xd020"
		);

		freqlo = MATH.DIVOUTFRACT0;
		freqhi = MATH.DIVOUTFRACT1;

		MATH.MULTINA0 = sample_rate_divisor >> 0;
		MATH.MULTINA1 = sample_rate_divisor >> 8;
		MATH.MULTINA2 = sample_rate_divisor >> 16;
		MATH.MULTINA3 = 0;

		MATH.MULTINB0 = freqlo;
		MATH.MULTINB1 = freqhi;
		MATH.MULTINB2 = 0;
		MATH.MULTINB3 = 0;

		poke(0xd724 + ch_ofs, MATH.MULTOUT2);																				// Pick results from output / 2^16
		poke(0xd725 + ch_ofs, MATH.MULTOUT3);
		poke(0xd726 + ch_ofs, 0);

		if (sample_loopstart[samplenum])
		{
			poke(0xd720 + ch_ofs, 0xc2);																					// Enable playback+ nolooping of channel 0, 8-bit, no unsigned samples
		}
		else
		{
			poke(0xd720 + ch_ofs, 0x82);																					// Enable playback+ nolooping of channel 0, 8-bit, no unsigned samples
		}
	}

	switch (effecthi)
	{
		case 0xf:																											// Speed / tempo
			if (effectlo < 0x20)																							// speed (00-1F) / tempo (20-FF)
			{
				tempo = RASTERS_PER_MINUTE / beats_per_minute / ROWS_PER_BEAT;
				tempo *= 6 / (effectlo & 0x1f);
				speed = tempo / NUMRASTERS;
				ticks = speed;
			}
			else																											// effect & 0x0ff >= 0x20
			{
				beats_per_minute = effectlo;
				tempo = RASTERS_PER_MINUTE / beats_per_minute / ROWS_PER_BEAT;
				speed = tempo / NUMRASTERS;
				ticks = speed;
			}
			break;
		case 0xc:																											// Channel volume
			poke(0xd729 + ch_ofs, effectlo);																				// CH0VOLUME
			poke(0xd71c + channel, effectlo);																				// CH0RVOL
			break;
	}

	poke(0xd711, 0b10010000);																								// Enable audio dma, enable bypass of audio mixer
}

void modplay_play_c()
{
	// play pattern row
	dma_lcopy(load_addr + song_offset + (current_pattern << 10) + (current_row << 4), (unsigned long)pattern_buffer, 16);

	modplay_playnote_c(0, &pattern_buffer[0 ]);
	modplay_playnote_c(1, &pattern_buffer[4 ]);
	modplay_playnote_c(2, &pattern_buffer[8 ]);
	modplay_playnote_c(3, &pattern_buffer[12]);

	current_row++;
	
	if(current_row > 0x3f)
	{
		current_row = 0x00;
		current_pattern_index++;
		if(current_pattern_index == numpatternindices)
			current_pattern_index = 0;
		current_pattern = song_pattern_list[current_pattern_index];
		current_row = 0;
	}
}

void modplay_init(unsigned long address)
{
	unsigned short i;
	unsigned char a;
	unsigned char numinstruments;

	load_addr = address;

	dma_lcopy(load_addr + 1080, (unsigned long)mod_tmpbuf, 4);						// Check if 15 or 31 instrument mode (M.K.)

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
			song_offset = 1084;														// case for 31 instruments
		}
	}

	for(i = 0; i < numinstruments; i++)
	{
		// 2 bytes - sample length in words. multiply by 2 for byte length
		// 1 byte  - lower fout bits for finetune, upper for
		// 1 byte  - volume for sample ($00-$40)
		// 2 bytes - repeat point in words
		// 2 bytes - repeat length in words
		dma_lcopy(load_addr + 0x14 + i * 30 + 22, (unsigned long)mod_tmpbuf, 8);	// Get instrument data for plucking
		sample_lengths   [i] = mod_tmpbuf[1] + (mod_tmpbuf[0] << 8);
		sample_lengths   [i] <<= 1;													// Redenominate instrument length into bytes
		sample_finetune  [i] = mod_tmpbuf[2];
		sample_vol       [i] = mod_tmpbuf[3];										// Instrument volume
		sample_loopstart [i] = mod_tmpbuf[5] + (mod_tmpbuf[4] << 8);				// Repeat start point and end point
		sample_looplen   [i] = mod_tmpbuf[7] + (mod_tmpbuf[6] << 8);
	}

	numpatternindices = lpeek(load_addr + 20 + numinstruments*30 + 0);
	song_loop_point = lpeek(load_addr + 20 + numinstruments*30 + 1);

	dma_lcopy(load_addr + 20 + numinstruments*30 + 2, (unsigned long)song_pattern_list, 128);
	for(i = 0; i < numpatternindices; i++)
	{
		if(song_pattern_list[i] > max_pattern)
			max_pattern = song_pattern_list[i];
	}

	sample_data_start = load_addr + song_offset + ((unsigned long)max_pattern + 1) * 1024;

	for(i = 0; i < MAX_INSTRUMENTS; i++)
	{
		sample_addr[i] = sample_data_start;
		sample_data_start += sample_lengths[i];
	}

	current_pattern_index = 0;
	current_pattern = song_pattern_list[0];
	current_row = 0;

	tempo = RASTERS_PER_MINUTE / beats_per_minute / ROWS_PER_BEAT;
	speed = tempo / NUMRASTERS;
	ticks = 1;

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
