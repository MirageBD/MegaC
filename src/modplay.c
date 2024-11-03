#include <calypsi/intrinsics6502.h>
#include <stdint.h>
#include "macros.h"
#include "registers.h"
#include "dma.h"

#define MAX_INSTRUMENTS						32
#define NUMRASTERS							(uint32_t)(2 * 312)
#define RASTERS_PER_SECOND					(uint32_t)(NUMRASTERS * 50)	// PAL 0-311, NTSC 0-262
#define RASTERS_PER_MINUTE					(uint32_t)(RASTERS_PER_SECOND * 60)
#define ROWS_PER_BEAT						4

#define NUM_SIGS							4
int8_t mod31_sigs[NUM_SIGS][4] =
{
	{ 0x4D, 0x2e, 0x4b, 0x2e },	// M.K.
	{ 0x4D, 0x21, 0x4b, 0x21 }, // M!K!
	{ 0x46, 0x4c, 0x54, 0x34 }, // FLT4
	{ 0x46, 0x4c, 0x54, 0x38 }  // FLT8
};

uint16_t periods[] =
{
	856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 453,
	428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240, 226,
	214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120, 113
};

// GLOBAL DATA

int32_t			sample_rate_divisor			= 1468299;
uint8_t			done						= 0;

// MOD DATA FOR 1 MOD ----------------------------------------

uint32_t		mod_addr;
uint16_t		mod_song_offset;
uint32_t		mod_sample_data;
uint8_t			mod_numpatterns;
uint8_t			mod_songlength;
uint8_t			mod_speed;
uint16_t		mod_tempo;
uint8_t			mod_patternlist				[128];
uint8_t			mod_tmpbuf[23];

uint8_t			currowdata					[16];			// data for current pattern

uint8_t			nextspeed;
uint8_t			nexttempo;

uint8_t			loop						= 1;

uint8_t			beats_per_minute			= 125;
uint8_t			pattern;
uint8_t			curpattern;
uint8_t			row;
uint8_t			currow;
uint8_t			song_loop_point;			// unused
uint16_t		top_addr;

// SAMPLE DATA FOR ALL INSTRUMENTS ---------------------------

uint16_t		sample_lengths				[MAX_INSTRUMENTS];
uint8_t			sample_finetune				[MAX_INSTRUMENTS];
uint8_t			sample_vol					[MAX_INSTRUMENTS];
uint16_t		sample_repeatpoint			[MAX_INSTRUMENTS];
uint16_t		sample_repeatlength			[MAX_INSTRUMENTS];
uint32_t		sample_addr					[MAX_INSTRUMENTS];

// CHANNEL DATA FOR ALL 4 CHANNELS ---------------------------

uint8_t			channel_sample				[4];
int8_t			channel_volume				[4];
int8_t			channel_tempvolume			[4];
uint32_t		channel_index				[4];			// used in 'retrigger note + x vblanks (ticks)', depends on offset
uint8_t			channel_repeat				[4];			// was bool, now 0-1 unsigned char
uint8_t			channel_stop				[4];			// was bool, now 0-1 unsigned char
uint8_t			channel_deltick				[4];
uint16_t		channel_period				[4];
uint16_t		channel_arp[3]				[4];
uint16_t		channel_portdest			[4];
uint16_t		channel_tempperiod			[4];
uint8_t			channel_portstep			[4];
uint8_t			channel_cut					[4];
int8_t			channel_retrig				[4];			// seems to be always 0 and never set again
uint8_t			channel_vibspeed			[4];
uint8_t			channel_vibwave				[4];
uint8_t			channel_vibpos				[4];
uint8_t			channel_vibdepth			[4];
uint8_t			channel_tremspeed			[4];
uint8_t			channel_tremwave			[4];
uint8_t			channel_trempos				[4];
uint8_t			channel_tremdepth			[4];
int8_t			channel_looppoint			[4];
int8_t			channel_loopcount			[4];
uint16_t		channel_offset				[4];
uint16_t		channel_offsetmem			[4];

// TEMP VALUES FOR PLAYNOTE ----------------------------------

uint8_t			samplenum;
uint8_t			effectdata;
uint8_t			effecthi;
uint8_t			periodlo;
uint8_t			periodhi;
uint8_t			freqlo;
uint8_t			freqhi;

uint8_t*		sample_address_ptr;
uint8_t			sample_address0;
uint8_t			sample_address1;
uint8_t			sample_address2;
uint8_t			sample_address3;

// -----------------------------------------------------------

uint8_t			globaltick;

// ------------------------------------------------------------------------------------

void preprocesseffects(uint8_t* data)
{
	if ((*(data+2) & 0x0f) == 0x0f) // set speed / tempo
	{
		uint8_t effectdata = *(data + 3);

		if(effectdata > 0x1f)
		{
			mod_tempo    = effectdata;
			nexttempo    = effectdata;
		}
		else
		{
			mod_speed = effectdata;
			nextspeed = effectdata;
		}
	}
}

void processnote(uint8_t channel, uint8_t *data)
{
	if(globaltick != 0)
		return;

	samplenum  = data[0] & 0xf0;
	samplenum |= data[2] >> 4;
	samplenum--;

	periodlo = data[1];
	periodhi = (data[0] & 0xf);

	effectdata = data[3];
	effecthi = (data[2] & 0xf);

	uint8_t ch_ofs = channel << 4;

	if((periodlo | periodhi) != 0)
	{
		poke(0xd720 + ch_ofs, 0x00);																						// Stop playback while loading new sample data

		sample_address_ptr = (uint8_t *)sample_addr + samplenum * 4;
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

		if (sample_repeatpoint[samplenum])																					// XXX - We should set base addr and top addr to the looping range, if the sample has one.
		{
			// start of loop
			poke(0xd721 + ch_ofs, (((uint32_t)sample_addr[samplenum] + 2 * sample_repeatpoint[samplenum]                                  ) >> 0 ) & 0xff);
			poke(0xd722 + ch_ofs, (((uint32_t)sample_addr[samplenum] + 2 * sample_repeatpoint[samplenum]                                  ) >> 8 ) & 0xff);
			poke(0xd723 + ch_ofs, (((uint32_t)sample_addr[samplenum] + 2 * sample_repeatpoint[samplenum]                                  ) >> 16) & 0xff);

			// Top addr
			poke(0xd727 + ch_ofs, (((uint16_t)sample_addr[samplenum] + 2 * (sample_repeatpoint[samplenum] + sample_repeatlength[samplenum] - 1)) >> 0 ) & 0xff);
			poke(0xd728 + ch_ofs, (((uint16_t)sample_addr[samplenum] + 2 * (sample_repeatpoint[samplenum] + sample_repeatlength[samplenum] - 1)) >> 8 ) & 0xff);
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

		if (sample_repeatpoint[samplenum])
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
			if(effectdata == 0)
			{
				done = 1;
				break;
			}
			if (effectdata > 0x1f)																							// speed (00-1F) / tempo (20-FF)
			{
				beats_per_minute = effectdata;
				mod_tempo = RASTERS_PER_MINUTE / beats_per_minute / ROWS_PER_BEAT;
				mod_speed = mod_tempo / NUMRASTERS;
			}
			else																											// effect & 0x0ff >= 0x20
			{
				mod_tempo = RASTERS_PER_MINUTE / beats_per_minute / ROWS_PER_BEAT;
				mod_tempo *= 6 / (effectdata & 0x1f);
				mod_speed = mod_tempo / NUMRASTERS;
			}
			break;
		case 0xc:																											// Channel volume
			poke(0xd729 + ch_ofs, effectdata);																				// CH0VOLUME
			poke(0xd71c + channel, effectdata);																				// CH0RVOL
			break;
	}

	poke(0xd711, 0b10010000);																								// Enable audio dma, enable bypass of audio mixer
}

void steptick()
{
	if(row == 64)
	{
		row = 0;
		pattern++;
	}

	if(pattern >= mod_songlength)
	{
		if(loop)
		{
			pattern      =    0;
			row          =    0;
			globaltick   =    0;
			mod_speed    =    6;
			nextspeed    =    6;
			mod_tempo    =  125;
			nexttempo    =  125;
		}
		else
		{
			done = 1;
			return;
		}
	}

	if(globaltick == 0)
	{
		dma_lcopy(mod_addr + mod_song_offset + (mod_patternlist[pattern] << 10) + (row << 4), (uint32_t)currowdata, 16);
		currow     = row;
		curpattern = pattern;

		preprocesseffects(&currowdata[0 ]);
		preprocesseffects(&currowdata[4 ]);
		preprocesseffects(&currowdata[8 ]);
		preprocesseffects(&currowdata[12]);

		mod_speed = nextspeed;
		mod_tempo = nexttempo;
	}

	processnote(0, &currowdata[0 ]);
	processnote(1, &currowdata[4 ]);
	processnote(2, &currowdata[8 ]);
	processnote(3, &currowdata[12]);

	globaltick++;
	if(globaltick == mod_speed)
	{
		row++;
		globaltick = 0;
	}
}

void modplay_init(uint32_t address)
{
	uint16_t i;
	uint8_t a, numinstruments;

	mod_addr = address;

	dma_lcopy(mod_addr + 1080, (uint32_t)mod_tmpbuf, 4);								// Check if 15 or 31 instrument mode (M.K.)

	mod_tmpbuf[4] = 0;
	numinstruments = 15;
	mod_song_offset = 1080 - (16 * 30);

	for(i = 0; i < NUM_SIGS; i++)
	{
		for(a = 0; a < 4; a++)
			if(mod_tmpbuf[a] != mod31_sigs[i][a])
				break;
		if(a == 4)
		{
			numinstruments = 31;
			mod_song_offset = 1084;														// case for 31 instruments
		}
	}

	for(i = 0; i < numinstruments; i++)
	{
		// 2 bytes - sample length in words. multiply by 2 for byte length
		// 1 byte  - lower fout bits for finetune, upper for
		// 1 byte  - volume for sample ($00-$40)
		// 2 bytes - repeat point in words
		// 2 bytes - repeat length in words
		dma_lcopy(mod_addr + 0x14 + i * 30 + 22, (uint32_t)mod_tmpbuf, 8);				// Get instrument data for plucking
		sample_lengths      [i] = mod_tmpbuf[1] + (mod_tmpbuf[0] << 8);
		sample_lengths      [i] <<= 1;													// Redenominate instrument length into bytes

		int8_t tempfinetune = mod_tmpbuf[2] & 0x0f;
		if(tempfinetune > 0x07)
			tempfinetune |= 0xf0;

		sample_finetune     [i] = tempfinetune;
		sample_vol          [i] = mod_tmpbuf[3];										// Instrument volume
		sample_repeatpoint  [i] = mod_tmpbuf[5] + (mod_tmpbuf[4] << 8);					// Repeat start point and end point
		sample_repeatlength [i] = mod_tmpbuf[7] + (mod_tmpbuf[6] << 8);
	}

	mod_songlength = lpeek(mod_addr + 20 + numinstruments*30 + 0);
	song_loop_point = lpeek(mod_addr + 20 + numinstruments*30 + 1);

	dma_lcopy(mod_addr + 20 + numinstruments*30 + 2, (uint32_t)mod_patternlist, 128);
	for(i = 0; i < mod_songlength; i++)
	{
		if(mod_patternlist[i] > mod_numpatterns)
			mod_numpatterns = mod_patternlist[i];
	}

	mod_sample_data = mod_addr + mod_song_offset + ((uint32_t)mod_numpatterns + 1) * 1024;

	for(i = 0; i < MAX_INSTRUMENTS; i++)
	{
		sample_addr[i] = mod_sample_data;
		mod_sample_data += sample_lengths[i];
	}

	pattern = 0;
	row = 0;

	mod_speed = 6;
	nextspeed = 6;
	mod_tempo = 125;
	nexttempo = 125;
	globaltick = 0;

	done = 0;

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
