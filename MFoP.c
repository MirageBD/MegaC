static uint32_t const	PAL_CLOCK		= 3546895;
static double const		SAMPLE_RATE		= 48000;
static double const		FINETUNE_BASE	= 1.0072382087;

typedef struct
{
	char		name[23];
	uint16_t	length;
	int8_t		finetune;
	uint8_t		volume;
	uint16_t	repeatpoint;
	uint16_t	repeatlength;
	int8_t*		sampledata;
} sample;

typedef struct
{
	sample*		sample;
	int8_t		volume;
	int8_t		tempvolume;
	uint32_t	index;			// used in 'retrigger note + x vblanks (ticks)', depends on offset in this struct
	bool		repeat;
	bool		stop;
	uint8_t		deltick;
	uint16_t	period;
	uint16_t	arp[3];
	uint16_t	portdest;
	uint16_t	tempperiod;
	uint8_t		portstep;
	uint8_t		cut;
	int8_t		retrig;			// seems to be always 0 and never set again
	uint8_t		vibspeed;
	uint8_t		vibwave;
	uint8_t		vibpos;
	uint8_t		vibdepth;
	uint8_t		tremspeed;
	uint8_t		tremwave;
	uint8_t		trempos;
	uint8_t		tremdepth;
	int8_t		looppoint;
	int8_t		loopcount;
	uint16_t	offset;
	uint16_t	offsetmem;
} channel;

uint16_t periods[] =
{
	856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 453,
	428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240, 226,
	214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120, 113
};

int16_t*	waves	[ 3];
int16_t		sine	[64];
int16_t		saw		[64];
int16_t		square	[64];

off_t		filelength;
bool		loop;

int			curpattern;
uint8_t*	currowdata;						// 4 channels * 4 bytes = 16 bytes. copy using DMA
bool		done;
bool		patternset;
double		ticktime;
double		nextticktime;
uint8_t		nexttempo;
uint8_t		nextspeed;
uint8_t		numsamples;
uint8_t		type;

// variables initialized in initsound
int			row;
int			currow;
int			pattern;
uint8_t		delcount;
uint8_t		globaltick;						// goes from 0 to mod_speed
bool		delset;
bool		inrepeat;
bool		addflag;						// used for emulating obscure Dxx bug

// current MOD variables
char		mod_name[21];
uint8_t*	mod_patterns;
uint8_t		mod_songlength;
sample*		mod_samples[31];
uint8_t		mod_patternlist[128];
uint32_t	mod_speed;
uint16_t	mod_tempo;
char		mod_magicstring[5];

int findperiod(uint16_t period)
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

static inline double calcrate(uint16_t period, int8_t finetune)
{
	return PAL_CLOCK / (round((double)period * pow(FINETUNE_BASE, -(double)finetune)));
}

void precalculatetables()
{
	waves[0] = sine;
	waves[1] = saw;
	waves[2] = square;

	double cursin;
	for(int i = 0; i < 64; i++)
	{
		cursin		= sin(i * 2 * M_PI / 64.0);
		sine[i]		= 255 * cursin;
		saw[i]		= 255 - (8 * i);
		square[i]	= (i < 32) ? 255 : -256;
	}
}

channel* initsound()
{
	precalculatetables();

	channel* channels	= malloc(                     4 * sizeof(channel));

	row			= 0;
	currow		= 0;
	pattern		= 0;
	delcount	= 0;
	globaltick	= 0;
	delset		= false;
	inrepeat	= false;
	addflag		= false;

	for(int i = 0; i < 4; i++)
	{
		channels[i].volume					= 0;
		channels[i].tempvolume				= 0;
		channels[i].deltick					= 0;
		channels[i].stop					= true;
		channels[i].repeat					= false;
		channels[i].period					= 0;
		channels[i].portdest				= 0;
		channels[i].tempperiod				= 0;
		channels[i].portstep				= 0;
		channels[i].offset					= 0;
		channels[i].offsetmem				= 0;
		channels[i].retrig					= 0;
		channels[i].vibwave					= 0;
		channels[i].tremwave				= 0;
		channels[i].vibpos					= 0;
		channels[i].trempos					= 0;
		channels[i].looppoint				= 0;
		channels[i].loopcount				= -1;
		channels[i].sample					= NULL;
	}

	return channels;
}

void renderpattern(uint8_t* patterndata)
{
	uint16_t period;
	uint8_t  tempsam;
	uint8_t* data;
	uint8_t  tempeffect;
	uint8_t  effectdata;

	for(int line = 0; line < 64; line++)
	{
		for(int chan = 0; chan < 4; chan++)
		{
			data		= patterndata + 16 * line + chan * 4;
			period		= (((uint16_t)((*data) & 0x0f)) << 8) | (uint16_t)(*(data + 1));
			tempsam		= ((((*data)) & 0xf0) | ((*(data + 2) >> 4) & 0x0f));
			tempeffect	= *(data + 2) & 0x0f;
			effectdata	= *(data + 3);
		}
	}

	return;
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
			ticktime     = 1 / (0.4 * effectdata);
			nextticktime = 1 / (0.4 * effectdata);
		}
		else
		{
			mod_speed = effectdata;
			nextspeed = effectdata;
		}
	}
}

void processnoteeffects(channel* c, uint8_t* data)
{
	uint8_t tempeffect = *(data + 2) & 0x0f;
	uint8_t effectdata = *(data + 3);

	switch(tempeffect)
	{
		case 0x00: // normal / arpeggio
			if(effectdata)
				c->tempperiod = c->arp[globaltick % 3];
			break;

		case 0x01: // slide up
			c->period -= effectdata;
			c->tempperiod = c->period;
			break;

		case 0x02: // slide down
			c->period += effectdata;
			c->tempperiod = c->period;
			break;

		case 0x05: // tone portamento + volume slide
			if(effectdata & 0xf0)
				c->volume += ((effectdata >> 4) & 0x0f); //slide up
			else
				c->volume -= effectdata; //slide down
			c->tempvolume = c->volume;
			// no break, exploit fallthrough

		case 0x03: // tone portamento
			if(abs(c->portdest - c->period) < c->portstep)
				c->period = c->portdest;
			else if(c->portdest > c->period)
				c->period += c->portstep;
			else
				c->period -= c->portstep;
			c->tempperiod = c->period;
			break;

		case 0x06: // vibrato + volume slide
			if(effectdata & 0xf0)
				c->volume += ((effectdata >> 4) & 0x0f); //slide up
			else
				c->volume -= effectdata; // slide down
			c->tempvolume = c->volume;
			// no break, exploit fallthrough

		case 0x04: // vibrato
			c->tempperiod  = c->period +	((c->vibdepth * waves[c->vibwave & 3][c->vibpos]) >> 7);
			c->vibpos     += c->vibspeed;
			c->vibpos     %= 64;
			break;

		case 0x07: // tremolo
			c->tempvolume  = c->volume +	((c->tremdepth * waves[c->tremwave & 3][c->trempos]) >> 6);
			c->trempos    += c->tremspeed;
			c->trempos    %= 64;
			break;

		case 0x0a: // volume slide
			if(effectdata & 0xf0)
				c->volume += ((effectdata>>4) & 0x0f); // slide up
			else
				c->volume -= effectdata; //slide down
			c->tempvolume = c->volume;
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
					c->vibwave = effectdata & 0x0f;
					break;

				case 0x50: // set finetune
				{
					int8_t tempfinetune = effectdata & 0x0f;
					if(tempfinetune > 0x07)
						tempfinetune |= 0xf0;
					c->sample->finetune = tempfinetune;
					break;
				}

				case 0x70: // set tremolo waveform (0 sine, 1 ramp down, 2 square)
					c->tremwave = effectdata & 0x0f;
					break;

				// There's no effect 0xE8

				case 0x90: // retrigger note + x vblanks (ticks)
					if(((effectdata & 0x0f) == 0) || (globaltick % (effectdata & 0x0f)) == 0)
						c->index = c->offset;
					break;

				case 0xc0: // cut from note + x vblanks
					if(globaltick == (effectdata & 0x0f))
						c->volume = 0;
					break;
			}
			break;
		}

		case 0x0f:
			if(effectdata == 0)
			{
				done = true;
				break;
			}
			if(effectdata > 0x1f)
			{
				nexttempo = effectdata;
				nextticktime = 1 / (0.4f * effectdata);
			}
			else nextspeed = effectdata;
			
			break;

		default:
			break;
	}
}

void processnote(channel* c, uint8_t* data, uint8_t offset, bool overwrite)
{
	uint8_t tempeffect = *(data + 2) & 0x0f;
	uint8_t effectdata = *(data + 3);
	
	if(globaltick == 0 && tempeffect == 0x0e && (effectdata & 0xf0) == 0xd0)
			c->deltick = (effectdata & 0x0f) % mod_speed;

	if(globaltick == c->deltick)
	{
		uint16_t period = (((uint16_t)((*data) & 0x0f)) << 8) | (uint16_t)(*(data + 1));
		uint8_t tempsam = ((((*data)) & 0xf0) | ((*(data + 2) >> 4) & 0x0f));

		if((period || tempsam) && !inrepeat)
		{
			if(tempsam)
			{
				c->stop = false;
				tempsam--;
				if(tempeffect != 0x03 && tempeffect != 0x05)
					c->offset = 0;
				c->sample = mod_samples[tempsam];
				c->volume = c->sample->volume;
				c->tempvolume = c->volume;
			}

			if(period)
			{
				if(tempeffect != 0x03 && tempeffect != 0x05)
				{
					if(tempeffect == 0x09)
					{
						if(effectdata)
							c->offsetmem = effectdata * 0x100;
						c->offset += c->offsetmem;
					}
					c->period		= period;
					c->tempperiod	= period;
					c->index		= c->offset;
					c->stop			= false;
					c->repeat		= false;
					c->vibpos		= 0;
					c->trempos		= 0;
				}
				c->portdest = period;
			}
		}

		switch(tempeffect)
		{
			case 0x00:
				if(effectdata)
				{
					c->period = c->portdest;
					int base = findperiod(c->period);
					if(base == -1)
					{
						c->arp[0] = c->period;
						c->arp[1] = c->period;
						c->arp[2] = c->period;
						break;
					}
					uint8_t step1 = base + ((effectdata >> 4) & 0x0f);
					uint8_t step2 = base + ((effectdata     ) & 0x0f);
					c->arp[0] = c->period;
					
					if(step1 > 35)
					{
						if(step1 == 36)
							c->arp[1] = 0;
						else
							c->arp[1] = periods[(step1 - 1) % 36];
					}
					else
					{
						c->arp[1] = periods[step1];
					}

					if(step2 > 35)
					{
						if(step2 == 36)
							c->arp[2] = 0;
						else
							c->arp[2] = periods[(step2 - 1) % 36];
					}
					else c->arp[2] = periods[step2];
				}
				break;

			case 0x03:
				if(effectdata)
					c->portstep = effectdata;
				break;

			case 0x04: // vibrato
				if(effectdata & 0x0f)
					c->vibdepth = (effectdata     ) & 0x0f;
				if(effectdata & 0xf0)
					c->vibspeed = (effectdata >> 4) & 0x0f;
				break;

			case 0x07: // tremolo
				if(effectdata)
				{
					c->tremdepth = (effectdata     ) & 0x0f;
					c->tremspeed = (effectdata >> 4) & 0x0f;
				}
				break;

			case 0x0b: // position jump
				if(currow == row)
					row = 0;
				pattern = effectdata;
				patternset = true;
				break;

			case 0x0c: // set volume
				if(effectdata > 64)
					c->volume = 64;
				else
					c->volume = effectdata;
				c->tempvolume = c->volume;
				break;

			case 0x0d: // row jump
				if(delcount)
					break;
				if(!patternset)
					pattern++;
				if(pattern >= mod_songlength)
					pattern = 0;
				row = (effectdata >> 4) * 10 + (effectdata & 0x0f);
				patternset = true;
				if(addflag)
					row++; // emulate protracker EEx + Dxx bug
				break;

			case 0x0e:
			{
				switch(effectdata & 0xf0)
				{
					case 0x10:
						c->period -= effectdata & 0x0f;
						c->tempperiod = c->period;
						break;

					case 0x20:
						c->period += effectdata & 0x0f;
						c->tempperiod = c->period;
						break;

					case 0x60: // jump to loop, play x times
						if(!(effectdata & 0x0f))
						{
							c->looppoint = row;
						}
						else if(effectdata & 0x0f)
						{
							if(c->loopcount == -1)
							{
								c->loopcount = (effectdata & 0x0f);
								row = c->looppoint;
							}
							else if(c->loopcount)
							{
								row = c->looppoint;
							}
							c->loopcount--;
						}
						break;

					case 0xa0:
						c->volume += (effectdata & 0x0f);
						c->tempvolume = c->volume;
						break;

					case 0xb0:
						c->volume -= (effectdata & 0x0f);
						c->tempvolume = c->volume;
						break;

					case 0xc0:
						c->cut = effectdata & 0x0f;
						break;

					case 0xe0: // delay pattern x notes
						if(!delset)
							delcount = effectdata & 0x0f;
						delset = true;
						addflag = true; // emulate bug that causes protracker to cause Dxx to jump too far when used in conjunction with EEx
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

		if(c->tempperiod == 0 || c->sample == NULL || c->sample->length == 0)
			c->stop = true;
	}
	else if(c->deltick == 0)
	{
		processnoteeffects(c, data);
	}

	if(c->retrig && globaltick == c->retrig - 1)
	{
		c->index	= 0;
		c->stop		= false;
		c->repeat	= false;
	}

	double conv_ratio;

	// RESAMPLE PER TICK

	int writesize = SAMPLE_RATE * ticktime;

	if(c->volume < 0)
		c->volume = 0;
	else if(c->volume > 64)
		c->volume = 64;

	if(c->tempvolume < 0)
		c->tempvolume = 0;
	else if(c->tempvolume > 64)
		c->tempvolume = 64;
	
	if(c->tempperiod > 856)
		c->tempperiod = 856;
	else if(c->tempperiod < 113)
		c->tempperiod = 113;

	// LV - CONVERTER STUFF WAS HAPPENING HERE

	if(globaltick == mod_speed - 1)
	{
		c->tempperiod = c->period;
		c->deltick = 0;
	}
}

void sampleparse(uint8_t* filearr, uint32_t start)
{
	for(int i = 0; i < numsamples; i++)
	{
		sample* s = malloc(sizeof(sample));
		mod_samples[i] = s;

		strncpy(s->name, (char*)filearr+20+(30*i), 22);
		s->name[22] = '\x00';

		s->length  = (uint16_t)*(filearr + 42 + (30 * i)) << 8;
		s->length |= (uint16_t)*(filearr + 43 + (30 * i));

		for(int j = 0; j < 22; j++)
		{
			if(!s->name[j])
				break;
			if(s->name[j] < 32)
				s->name[j] = 32;
		}

		if (s->length != 0)
		{
			int8_t tempfinetune = filearr[44 + 30 * i] & 0x0f;
			if(tempfinetune > 0x07)
				tempfinetune |= 0xf0;

			s->finetune = tempfinetune;
			s->volume = filearr[45 + 30 * i];

			s->repeatpoint   = (uint16_t)*(filearr + 46 + (30 * i)) << 8;
			s->repeatpoint  |= (uint16_t)*(filearr + 47 + (30 * i));

			s->repeatlength  = (uint16_t)*(filearr + 48 + (30 * i)) << 8;
			s->repeatlength |= (uint16_t)*(filearr + 49 + (30 * i));

			int copylen = (s->length) * 2;
			s->sampledata = malloc(copylen * sizeof(int8_t));

			if ((start + copylen) > filelength)
				abort();

			memcpy(s->sampledata, (int8_t*)(filearr + start), copylen);
			start += copylen;
		}
	}

	return;
}

void steptick(channel* cp)
{
	if(row == 64)
	{
		row = 0;
		if(pattern == curpattern)
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
			ticktime     = 0.02;
			nextticktime = 0.02;
		}
		else
		{
			done = true;
			return;
		}
	}

	if(globaltick == 0)
	{
		if(pattern != curpattern)
			renderpattern(mod_patterns + 1024 * mod_patternlist[pattern]);

		patternset = false;
		currowdata = mod_patterns + ((mod_patternlist[pattern]) * 1024) + (16 * row);
		currow     = row;
		curpattern = pattern;

		preprocesseffects(currowdata +  0);
		preprocesseffects(currowdata +  4);
		preprocesseffects(currowdata +  8);
		preprocesseffects(currowdata + 12);

		mod_speed = nextspeed;
		mod_tempo = nexttempo;
		ticktime = nextticktime;
	}

	processnote(&cp[0], currowdata +  0, 0,  true);
	processnote(&cp[1], currowdata +  4, 1,  true);
	processnote(&cp[2], currowdata +  8, 1, false);
	processnote(&cp[3], currowdata + 12, 0, false);

	globaltick++;
	if(globaltick == mod_speed)
	{
		if(delcount)
		{
			inrepeat = true;
			delcount--;
		}
		else
		{
			delset   = false;
			addflag  = false;
			inrepeat = false;

			if(currow == row)
				row++;
		}

		globaltick = 0;
	}
}

void modparse(FILE* f)
{
	uint8_t* filearr = malloc(filelength * sizeof(uint8_t));
	int seek = 0;
	int c;
	
	while((c = fgetc(f)) != EOF)
		filearr[seek++] = (uint8_t)c;

	modfile* m = malloc(sizeof(modfile));
	
	strncpy((char*)mod_name, (char*)filearr, 20);
	mod_name[20] = '\x00';

	memcpy(mod_magicstring, filearr + 1080, 4);
	mod_magicstring[4] = '\x00';
	if(strcmp(mod_magicstring, "M.K.") && strcmp(mod_magicstring, "4CHN"))
		type = 1;	// Not a 31 instrument 4 channel MOD file. May not be playable
	else
		type = 0;

	numsamples = type ? 15 : 31;

	if (type == 0)
		mod_songlength = filearr[950];
	else
		mod_songlength = filearr[470];

	if (type == 0)
		memcpy(mod_patternlist, filearr + 952, 128);
	else
		memcpy(mod_patternlist, filearr + 472, 128);

	int max = 0;
	for(int i = 0; i < 128; i++)
	{
		if(mod_patternlist[i] > max)
			max = mod_patternlist[i];
	}

	uint32_t len = (uint32_t)(1024 * (max + 1)); // 1024 = size of pattern
	mod_patterns = malloc(len);

	uint16_t size;
	if(type == 0)
		size = 1084;
	else
		size = 600;

	memcpy(mod_patterns, filearr + size, len);
	
	sampleparse(filearr, len + size);
	
	mod_speed    =    6; // default speed = 6
	nextspeed    =    6;
	mod_tempo    =  125;
	nexttempo    =  125;
	ticktime     = 0.02;
	nextticktime = 0.02;

	free(filearr);
}
