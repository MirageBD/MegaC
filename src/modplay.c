#include <calypsi/intrinsics6502.h>
#include <stdint.h>
#include "macros.h"
#include "registers.h"
#include "dma.h"

/*
																							NoteEffects		Note

	0 - Normal play or Arpeggio             0xy : x-first halfnote add, y-second			x				x
	1 - Slide Up                            1xx : upspeed									x				-
	2 - Slide Down                          2xx : downspeed									x				-
	3 - Tone Portamento                     3xx : up/down speed								x				x
	4 - Vibrato                             4xy : x-speed,   y-depth						x				x
	5 - Tone Portamento + Volume Slide      5xy : x-upspeed, y-downspeed					x				-
	6 - Vibrato + Volume Slide              6xy : x-upspeed, y-downspeed					x				-
	7 - Tremolo                             7xy : x-speed,   y-depth						x				x
	8 - NOT USED																			-				-
	9 - Set SampleOffset                    9xx : offset (23 -> 2300)						-				-
	A - VolumeSlide                         Axy : x-upspeed, y-downspeed					x				-
	B - Position Jump                       Bxx : songposition								-				x
	C - Set Volume                          Cxx : volume, 00-40								-				x
	D - Pattern Break                       Dxx : break position in next patt				-				x
	E - E-Commands                          Exy : see below...								x				x
	F - Set Speed                           Fxx : speed (00-1F) / tempo (20-FF)				x				-
*/

#define MAX_INSTRUMENTS						32
#define NUMRASTERS							(uint32_t)(2 * 312)
#define RASTERS_PER_SECOND					(uint32_t)(NUMRASTERS * 50)	// PAL 0-311, NTSC 0-262
#define RASTERS_PER_MINUTE					(uint32_t)(RASTERS_PER_SECOND * 60)
#define ROWS_PER_BEAT						4

#define NUM_SIGS							4
int8_t mod31_sigs[NUM_SIGS][4] =
{
	{ 0x4d, 0x2e, 0x4b, 0x2e },	// M.K.
	{ 0x4d, 0x21, 0x4b, 0x21 }, // M!K!
	{ 0x46, 0x4c, 0x54, 0x34 }, // FLT4
	{ 0x46, 0x4c, 0x54, 0x38 }  // FLT8
};

uint16_t periods[] =
{
	856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 453,
	428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240, 226,
	214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120, 113,
};

int16_t	sine[64] =
{
    0,   24,   49,   74,   97,  120,  141,  161,  180,  197,  212,  224,  235,  244,  250,  253,
  254,  253,  250,  244,  235,  224,  212,  197,  180,  161,  141,  120,   97,   74,   49,   24,
    0,  -24,  -49,  -74,  -97, -120, -141, -161, -180, -197, -212, -224, -235, -244, -250, -253,
 -254, -253, -250, -244, -235, -224, -212, -197, -180, -161, -141, -120,  -97,  -74,  -49,  -24,
};

int16_t	saw[64] =
{
  255,  247,  239,  231,  223,  215,  207,  199,  191,  183,  175,  167,  159,  151,  143,  135,
  127,  119,  111,  103,   95,   87,   79,   71,   63,   55,   47,   39,   31,   23,   15,    7,
   -1,   -9,  -17,  -25,  -33,  -41,  -49,  -57,  -65,  -73,  -81,  -89,  -97, -105, -113, -121,
 -129, -137, -145, -153, -161, -169, -177, -185, -193, -201, -209, -217, -225, -233, -241, -249,
};

int16_t	square[64] =
{
  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,
  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,
 -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256,
 -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256, -256,
};

int16_t* waves [3] = { sine, saw, square };

uint8_t enabled_channels[4] = { 1, 1, 1, 1 };

// GLOBAL DATA

int32_t			sample_rate_divisor			= 1468299;
// int32_t			sample_rate_divisor			= 1400000;

uint8_t			done						= 0;
uint8_t			patternset;

// variables initialized in initsound
uint8_t			row;
uint8_t			currow;
uint8_t			pattern;
uint8_t			delcount;
uint8_t			globaltick;
uint8_t			delset;
uint8_t			inrepeat;
uint8_t			addflag;

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
uint8_t			curpattern;
uint8_t			song_loop_point;			// unused
uint16_t		top_addr;

uint8_t			arpeggiocounter;

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

uint8_t			freqlo;
uint8_t			freqhi;

uint8_t*		sample_address_ptr;
uint8_t			sample_address0;
uint8_t			sample_address1;
uint8_t			sample_address2;
uint8_t			sample_address3;

// ------------------------------------------------------------------------------------

int8_t findperiod(uint16_t period)
{
	if(period > 856 || period < 113)
		return -1;

	uint8_t upper = 35;
	uint8_t lower = 0;
	uint8_t mid;

	while(upper >= lower)
	{
		mid = (upper + lower) / 2;
		if(periods[mid] == period)
			return mid;
		else if(periods[mid] > period)
			lower = mid + 1;
		else
			upper = mid - 1;
	}

	return -1;
}

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
	uint8_t tempsam = ((((*data)) & 0xf0) | ((*(data + 2) >> 4) & 0x0f));

	uint16_t period = (((uint16_t)((*data) & 0x0f)) << 8) | (uint16_t)(*(data + 1));

	uint8_t effectdata = *(data + 3);
	uint8_t tempeffect = *(data + 2) & 0x0f;

	// EDx : delay note x vblanks
	if(globaltick == 0 && tempeffect == 0x0e && (effectdata & 0xf0) == 0xd0)
		channel_deltick[channel] = (effectdata & 0x0f) % mod_speed;

	uint8_t ch_ofs = channel << 4;

	uint8_t triggersample = 0; // NOT IN ORIGINAL SOURCE - FIND BETTER WAY OF HANDLING THIS!!!

	if(globaltick == channel_deltick[channel]) // 0 if no note delay
	{
		if((period || tempsam) && !inrepeat)
		{
			triggersample = 1;

			if(tempsam)
			{
				channel_stop[channel] = 0;
				tempsam--;
				if(tempeffect != 0x03 && tempeffect != 0x05)
					channel_offset[channel] = 0;
				channel_sample[channel] = tempsam;
				channel_volume[channel] = sample_vol[tempsam];
				channel_tempvolume[channel] = channel_volume[channel];
			}

			if(period)
			{
				if(tempeffect != 0x03 && tempeffect != 0x05)
				{
					if(tempeffect == 0x09)
					{
						if(effectdata)
							channel_offsetmem[channel] = effectdata * 0x100;
						channel_offset[channel] += channel_offsetmem[channel];
					}
					channel_period    [channel]	= period;
					channel_tempperiod[channel]	= period;
					channel_index     [channel] = channel_offset[channel];
					channel_stop      [channel]	= 0;
					channel_repeat    [channel]	= 0;
					channel_vibpos    [channel]	= 0;
					channel_trempos   [channel]	= 0;
				}
				channel_portdest[channel] = period;
			}
		}

		switch (tempeffect)
		{
			case 0x00: // Normal play or Arpeggio
				if(effectdata)
				{
					channel_period[channel] = channel_portdest[channel];
					int8_t base = findperiod(channel_period[channel]);

					if(base == -1)
					{
						channel_arp[0][channel] = channel_period[channel];
						channel_arp[1][channel] = channel_period[channel];
						channel_arp[2][channel] = channel_period[channel];
						break;
					}

					arpeggiocounter = 0;

					uint8_t step1 = base + ((effectdata >> 4) & 0x0f);
					uint8_t step2 = base + ((effectdata     ) & 0x0f);

					channel_arp[0][channel] = channel_period[channel];

					if(step1 > 35)
					{
						if(step1 == 36)
							channel_arp[1][channel] = 0;
						else
							channel_arp[1][channel] = periods[(step1 - 1) % 36];
					}
					else
					{
						channel_arp[1][channel] = periods[step1];
					}

					if(step2 > 35)
					{
						if(step2 == 36)
							channel_arp[2][channel] = 0;
						else
							channel_arp[2][channel] = periods[(step2 - 1) % 36];
					}
					else channel_arp[2][channel] = periods[step2];
				}

				break;

			case 0x03: // Tone Portamento
				if(effectdata)
					channel_portstep[channel] = effectdata;
				break;

			case 0x04: // vibrato
				if(effectdata & 0x0f)
					channel_vibdepth[channel] = (effectdata     ) & 0x0f;
				if(effectdata & 0xf0)
					channel_vibspeed[channel] = (effectdata >> 4) & 0x0f;
				break;

			case 0x07: // tremolo
				if(effectdata)
				{
					channel_tremdepth[channel] = (effectdata     ) & 0x0f;
					channel_tremspeed[channel] = (effectdata >> 4) & 0x0f;
				}
				break;

			case 0x0b: // position jump
				if(currow == row)
					row = 0;
				pattern = effectdata;
				patternset = 1;
				break;

			case 0xc: // set volume
				if(effectdata > 63)
					channel_volume[channel] = 63;
				else
					channel_volume[channel] = effectdata;
				channel_tempvolume[channel] = channel_volume[channel];
				break;

			case 0x0d: // row jump
				if(delcount)
					break;
				if(!patternset)
					pattern++;
				if(pattern >= mod_songlength)
					pattern = 0;
				row = (effectdata >> 4) * 10 + (effectdata & 0x0f);
				patternset = 1;
				if(addflag)
					row++; // emulate protracker EEx + Dxx bug
				break;

			case 0x0e:
			{
				switch(effectdata & 0xf0)
				{
					case 0x10:
						channel_period[channel] -= effectdata & 0x0f;
						channel_tempperiod[channel] = channel_period[channel];
						break;

					case 0x20:
						channel_period[channel] += effectdata & 0x0f;
						channel_tempperiod[channel] = channel_period[channel];
						break;

					case 0x60: // jump to loop, play x times
						if(!(effectdata & 0x0f))
						{
							channel_looppoint[channel] = row;
						}
						else if(effectdata & 0x0f)
						{
							if(channel_loopcount[channel] == -1)
							{
								channel_loopcount[channel] = (effectdata & 0x0f);
								row = channel_looppoint[channel];
							}
							else if(channel_loopcount[channel])
							{
								row = channel_looppoint[channel];
							}
							channel_loopcount[channel]--;
						}
						break;

					case 0xa0:
						channel_volume[channel] += (effectdata & 0x0f);
						channel_tempvolume[channel] = channel_volume[channel];
						break;

					case 0xb0:
						channel_volume[channel] -= (effectdata & 0x0f);
						channel_tempvolume[channel] = channel_volume[channel];
						break;

					case 0xc0:
						channel_cut[channel] = effectdata & 0x0f;
						break;

					case 0xe0: // delay pattern x notes
						if(!delset)
							delcount = effectdata & 0x0f;
						delset = 1;
						addflag = 1; // emulate bug that causes protracker to cause Dxx to jump too far when used in conjunction with EEx
						break;

					case 0xf0:
						// c->funkspeed = funktable[effectdata & 0x0f];
						break;

					default:
						break;
				}
			}

			default:
				break;
		}

		if(channel_tempperiod[channel] == 0 /* || c->sample[channel] == NULL || c->sample->length == 0 */)
			channel_stop[channel] = 1;
	}
	else if(channel_deltick[channel] == 0)
	{
		switch (tempeffect)
		{
			case 0x00: // normal / arpeggio
				if(effectdata)
					channel_tempperiod[channel] = channel_arp[arpeggiocounter % 3][channel];
				break;

			case 0x01: // slide up
				channel_period[channel] -= effectdata;
				channel_tempperiod[channel] = channel_period[channel];
				break;

			case 0x02: // slide down
				channel_period[channel] += effectdata;
				channel_tempperiod[channel] = channel_period[channel];
				break;

			case 0x05: // tone portamento + volume slide
				if(effectdata & 0xf0)
					channel_volume[channel] += ((effectdata >> 4) & 0x0f); //slide up
				else
					channel_volume[channel] -= effectdata; //slide down
				channel_tempvolume[channel] = channel_volume[channel];
				// no break, exploit fallthrough

			case 0x03: // tone portamento
				if(channel_portdest[channel] > channel_period[channel])
				{
					channel_period[channel] += channel_portstep[channel];
					if(channel_period[channel] > channel_portdest[channel])
						channel_period[channel] = channel_portdest[channel];
				}
				else if(channel_portdest[channel] < channel_period[channel])
				{
					channel_period[channel] -= channel_portstep[channel];
					if(channel_period[channel] < channel_portdest[channel])
						channel_period[channel] = channel_portdest[channel];
				}
				channel_tempperiod[channel] = channel_period[channel];
				break;

			case 0x06: // vibrato + volume slide
				if(effectdata & 0xf0)
					channel_volume[channel] += ((effectdata >> 4) & 0x0f); //slide up
				else
					channel_volume[channel] -= effectdata; // slide down
				channel_tempvolume[channel] = channel_volume[channel];
				// no break, exploit fallthrough

			case 0x04: // vibrato
				channel_tempperiod[channel]  = channel_period[channel] + ((channel_vibdepth[channel] * waves[channel_vibwave[channel] & 3][channel_vibpos[channel]]) >> 7);
				channel_vibpos[channel]     += channel_vibspeed[channel];
				channel_vibpos[channel]     %= 64;
				break;

			case 0x07: // tremolo
				channel_tempvolume[channel]  = channel_volume[channel] + ((channel_tremdepth[channel] * waves[channel_tremwave[channel] & 3][channel_trempos[channel]]) >> 6);
				channel_trempos[channel]    += channel_tremspeed[channel];
				channel_trempos[channel]    %= 64;
				break;

			case 0x0a: // volume slide
				if(effectdata & 0xf0)
					channel_volume[channel] += ((effectdata>>4) & 0x0f); // slide up
				else
					channel_volume[channel] -= effectdata; //slide down
				channel_tempvolume[channel] = channel_volume[channel];
				break;

			case 0x0e: // E commands
			{
				switch(effectdata & 0xf0)
				{
					case 0x00: // set filter (0 on, 1 off)
						break;

					case 0x30: // glissando control (0 off, 1 on, use with tone portamento)
						break;

					case 0x40: // set vibrato waveform (0 sine, 1 ramp down, 2 square)
						channel_vibwave[channel] = effectdata & 0x0f;
						break;

					case 0x50: // set finetune
					{
						int8_t tempfinetune = effectdata & 0x0f;
						if(tempfinetune > 0x07)
							tempfinetune |= 0xf0;
						sample_finetune[channel_sample[channel]] = tempfinetune;
						break;
					}

					case 0x70: // set tremolo waveform (0 sine, 1 ramp down, 2 square)
						channel_tremwave[channel] = effectdata & 0x0f;
						break;

					// There's no effect 0xE8

					case 0x90: // retrigger note + x vblanks (ticks)
						if(((effectdata & 0x0f) == 0) || (globaltick % (effectdata & 0x0f)) == 0)
							channel_index[channel] = channel_offset[channel];
						break;

					case 0xc0: // cut from note + x vblanks
						if(globaltick == (effectdata & 0x0f))
							channel_volume[channel] = 0;
						break;
				}
				break;
			}

			case 0xf: // Speed / tempo
				if(effectdata == 0)
				{
					done = 1;
					break;
				}
				if (effectdata < 0x20) // speed (00-1F) / ticks per row (normally 6)
				{
					mod_speed = (effectdata & 0x1f);
				}
				else // tempo (20-FF)
				{
					//beats_per_minute = effectdata;
					// // mod_tempo *= 6 / (effectdata & 0x1f);
					//mod_tempo = RASTERS_PER_MINUTE / beats_per_minute / ROWS_PER_BEAT;
					//mod_speed = mod_tempo / NUMRASTERS;
				}
				break;
		}
	}

	if     (channel_volume[channel] < 0)		channel_volume[channel] = 0;
	else if(channel_volume[channel] > 63)		channel_volume[channel] = 63;

	if     (channel_tempvolume[channel] < 0)	channel_tempvolume[channel] = 0;
	else if(channel_tempvolume[channel] > 63)	channel_tempvolume[channel] = 63;
	
	if     (channel_tempperiod[channel] > 856)	channel_tempperiod[channel] = 856;
	else if(channel_tempperiod[channel] < 113)	channel_tempperiod[channel] = 113;

	if(globaltick == mod_speed - 1)
	{
		channel_tempperiod[channel] = channel_period[channel];
		channel_deltick[channel] = 0;
	}

	// SET FREQUENCY

	MATH.MULTINA0 = 0xff;																// calculate frequency // freq = 0xFFFFL / period
	MATH.MULTINA1 = 0xff;
	MATH.MULTINA2 = 0;
	MATH.MULTINA3 = 0;

	MATH.MULTINB0 = (channel_tempperiod[channel] >> 0) & 0xff;
	MATH.MULTINB1 = (channel_tempperiod[channel] >> 8) & 0xff;
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

	poke(0xd724 + ch_ofs, MATH.MULTOUT2);												// Pick results from output / 2^16
	poke(0xd725 + ch_ofs, MATH.MULTOUT3);
	poke(0xd726 + ch_ofs, 0);

	// SET VOLUME

	if(channel_stop[channel])
	{
		poke(0xd729 + ch_ofs,  0);														// CH0VOLUME
		poke(0xd71c + channel, 0);														// CH0RVOL
	}
	else
	{
		poke(0xd729 + ch_ofs,  channel_tempvolume[channel]);							// CH0VOLUME
		poke(0xd71c + channel, channel_tempvolume[channel]);							// CH0RVOL
	}

	// TRIGGER SAMPLE

	uint8_t curchansamp = channel_sample[channel];

	if(globaltick == 0 && triggersample == 1 && !channel_stop[channel])
	{
		if(enabled_channels[channel] == 0)
			return;

		poke(0xd720 + ch_ofs, 0x00);													// Stop playback while loading new sample data

		uint32_t sample_adr = sample_addr[curchansamp] + channel_offset[channel];

		sample_address0 = (sample_adr >>  0) & 0xff;
		sample_address1 = (sample_adr >>  8) & 0xff;
		sample_address2 = (sample_adr >> 16) & 0xff;
		sample_address3 = (sample_adr >> 24) & 0xff;

		poke(0xd721 + ch_ofs, sample_address0);											// Load sample address into base and current addr
		poke(0xd722 + ch_ofs, sample_address1);
		poke(0xd723 + ch_ofs, sample_address2);
		poke(0xd72a + ch_ofs, sample_address0);
		poke(0xd72b + ch_ofs, sample_address1);
		poke(0xd72c + ch_ofs, sample_address2);

		top_addr = sample_addr[curchansamp] + sample_lengths[curchansamp];						// Sample top address
		poke(0xd727 + ch_ofs, (top_addr >> 0) & 0xff);
		poke(0xd728 + ch_ofs, (top_addr >> 8) & 0xff);

		if(sample_repeatpoint[curchansamp])													// Set base addr and top addr to the looping range, if the sample has one.
		{
			// start of loop
			poke(0xd721 + ch_ofs, (((uint32_t)sample_addr[curchansamp] + 2 * sample_repeatpoint[curchansamp]                                  ) >> 0 ) & 0xff);
			poke(0xd722 + ch_ofs, (((uint32_t)sample_addr[curchansamp] + 2 * sample_repeatpoint[curchansamp]                                  ) >> 8 ) & 0xff);
			poke(0xd723 + ch_ofs, (((uint32_t)sample_addr[curchansamp] + 2 * sample_repeatpoint[curchansamp]                                  ) >> 16) & 0xff);

			// Top addr
			poke(0xd727 + ch_ofs, (((uint16_t)sample_addr[curchansamp] + 2 * (sample_repeatpoint[curchansamp] + sample_repeatlength[curchansamp] - 1)) >> 0 ) & 0xff);
			poke(0xd728 + ch_ofs, (((uint16_t)sample_addr[curchansamp] + 2 * (sample_repeatpoint[curchansamp] + sample_repeatlength[curchansamp] - 1)) >> 8 ) & 0xff);
		}

		if (sample_repeatpoint[curchansamp])
			poke(0xd720 + ch_ofs, 0xc2);												// Enable playback+ nolooping of channel 0, 8-bit, no unsigned samples
		else
			poke(0xd720 + ch_ofs, 0x82);												// Enable playback+ nolooping of channel 0, 8-bit, no unsigned samples

		poke(0xd711, 0b10010000);														// Enable audio dma, enable bypass of audio mixer
	}
}

void steptick()
{
	arpeggiocounter++;

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
		patternset = 0;
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
		if(delcount)
		{
			inrepeat = 1;
			delcount--;
		}
		else
		{
			delset   = 0;
			addflag  = 0;
			inrepeat = 0;

			if(currow == row)
				row++;
		}

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

	row			= 0;
	currow		= 0;
	pattern		= 0;
	delcount	= 0;
	globaltick	= 0;
	delset		= 0;
	inrepeat	= 0;
	addflag		= 0;

	mod_speed = 6;
	nextspeed = 6;
	mod_tempo = 125;
	nexttempo = 125;

	done = 0;

	for(i = 0; i < 4; i++)
	{
		channel_volume				[i] = 0;
		channel_tempvolume			[i] = 0;
		channel_deltick				[i] = 0;
		channel_stop				[i] = 1;
		channel_repeat				[i] = 0;
		channel_period				[i] = 0;
		channel_portdest			[i] = 0;
		channel_tempperiod			[i] = 0;
		channel_portstep			[i] = 0;
		channel_offset				[i] = 0;
		channel_offsetmem			[i] = 0;
		channel_retrig				[i] = 0;
		channel_vibwave				[i] = 0;
		channel_tremwave			[i] = 0;
		channel_vibpos				[i] = 0;
		channel_trempos				[i] = 0;
		channel_looppoint			[i] = 0;
		channel_loopcount			[i] = -1;
		channel_sample				[i] = 0;

		// channel_index			[i] = 0;
		// channel_arp[3]			[i] = 0;
		// channel_cut				[i] = 0;
		// channel_vibspeed			[i] = 0;
		// channel_vibdepth			[i] = 0;
		// channel_tremspeed		[i] = 0;
		// channel_tremdepth		[i] = 0;
	}

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
