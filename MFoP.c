static uint32_t const	PAL_CLOCK		= 3546895;
static double const		SAMPLE_RATE		= 48000;
static double const		FINETUNE_BASE	= 1.0072382087;

uint16_t periods[] =
{
	856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 453,
	428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240, 226,
	214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120, 113
};

uint8_t		funktable[] = { 0, 5, 6, 7, 8, 10, 11, 13, 16, 19, 22, 26, 32, 43, 64, 128 };

int16_t*	waves	[ 3];
int16_t		sine	[64];
int16_t		saw		[64];
int16_t		square	[64];

off_t		filelength;
bool		loop;
bool		headphones;
float*		audiobuf;
float*		mixbuf;

int			pattern;
int			row;
int			currow;
int			curpattern;
bool		addflag; // used for emulating obscure Dxx bug
uint8_t*	curdata;
bool		done;
uint8_t		globaltick;
bool		patternset;
uint8_t		delcount;
bool		delset;
bool		inrepeat;
double		ticktime;
double		nextticktime;
uint8_t		nexttempo;
uint8_t		nextspeed;
uint8_t		numsamples;
uint8_t		type;

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
	uint32_t	index;
	float*		buffer;
	float*		resampled;
	uint32_t	increment;
	bool		repeat;
	bool		stop;
	uint8_t		deltick;
	uint16_t	period;
	uint16_t	arp[3];
	uint16_t	portdest;
	uint16_t	tempperiod;
	uint8_t		portstep;
	uint8_t		cut;
	SRC_STATE*	converter;
	SRC_DATA*	cdata;
	int8_t		retrig;
	uint8_t		vibspeed;
	uint8_t		vibwave;
	uint8_t		vibpos;
	uint8_t		vibdepth;
	uint8_t		tremspeed;
	uint8_t		tremwave;
	uint8_t		trempos;
	uint8_t		tremdepth;
	uint8_t		funkcounter;
	uint8_t		funkpos;
	uint8_t		funkspeed;
	int8_t		looppoint;
	int8_t		loopcount;
	uint16_t	offset;
	uint16_t	offsetmem;
	float		error;
} channel;

typedef struct
{
	char		name[21];
	uint8_t*	patterns;
	uint8_t		songlength;
	sample*		samples[31];
	uint8_t		patternlist[128];
	uint32_t	speed;
	uint16_t	tempo;
	char		magicstring[5];
} modfile;

modfile* gm;
channel* gcp;

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
	mixbuf				= malloc(0.08 * 2 * SAMPLE_RATE * sizeof(float  ));
	audiobuf			= malloc(0.08 * 2 * SAMPLE_RATE * sizeof(float  ));

	for(int i = 0; i < 4; i++)
	{
		channels[i].error					= 0;
		channels[i].volume					= 0;
		channels[i].tempvolume				= 0;
		channels[i].deltick					= 0;
		channels[i].increment				= 0.0f;
		channels[i].buffer					= malloc(PAL_CLOCK * sizeof(float));
		channels[i].resampled				= malloc(0.08 * SAMPLE_RATE * sizeof(float));
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
		channels[i].funkcounter				= 0;
		channels[i].funkpos					= 0;
		channels[i].funkspeed				= 0;
		channels[i].looppoint				= 0;
		channels[i].loopcount				= -1;
		channels[i].sample					= NULL;
		channels[i].converter				= src_new(SRC_LINEAR, 1, &libsrc_error);
		channels[i].cdata					= malloc(sizeof(SRC_DATA));
		channels[i].cdata->data_in			= channels[i].buffer;
		channels[i].cdata->data_out			= channels[i].resampled;
		channels[i].cdata->output_frames	= SAMPLE_RATE * 0.02;
		channels[i].cdata->end_of_input		= 0;
		row			= 0;
		currow		= 0;
		pattern		= 0;
		delcount	= 0;
		globaltick	= 0;
		delset		= false;
		inrepeat	= false;
		addflag		= false;
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
	if ((*(data+2) & 0x0f) == 0x0f) // set speed/tempo
	{
		uint8_t effectdata = *(data + 3);

		if(effectdata > 0x1f)
		{
			gm->tempo    = effectdata;
			nexttempo    = effectdata;
			ticktime     = 1 / (0.4 * effectdata);
			nextticktime = 1 / (0.4 * effectdata);
		}
		else
		{
			gm->speed = effectdata;
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
			if(effectdata) c->tempperiod = c->arp[globaltick % 3];
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
			c->deltick = (effectdata & 0x0f) % gm->speed;

	if(globaltick == c->deltick)
	{
		uint16_t period = (((uint16_t)((*data) & 0x0f)) << 8) | (uint16_t)(*(data + 1));
		uint8_t tempsam = ((((*data)) & 0xf0) | ((*(data + 2) >> 4) & 0x0f));

		if((period || tempsam) && !inrepeat)
		{
			if(tempsam)
			{
				c->error = 0;
				c->stop = false;
				tempsam--;
				if(tempeffect != 0x03 && tempeffect != 0x05)
					c->offset = 0;
				c->sample = gm->samples[tempsam];
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
					c->error		= 0;
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
				if(pattern >= gm->songlength)
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
						c->funkspeed = funktable[effectdata & 0x0f];
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

	c->cdata->output_frames = ticktime * SAMPLE_RATE;

	if(c->stop)
	{
		// write empty frame

		conv_ratio = 1.0;
		c->cdata->src_ratio = conv_ratio;
		src_set_ratio(c->converter, conv_ratio);
		c->cdata->input_frames = ticktime * SAMPLE_RATE;

		for(int i = 0; i < ticktime * SAMPLE_RATE; i++)
			c->buffer[i] = 0.0f;
	}
	else
	{
		// write non-empty frame to buffer to be interpolated

		double rate = calcrate(c->tempperiod, c->sample->finetune);
		conv_ratio = SAMPLE_RATE / rate;
		c->cdata->src_ratio = conv_ratio;
		src_set_ratio(c->converter, conv_ratio);
		c->cdata->input_frames = ticktime * rate;

		c->funkcounter += c->funkspeed;
		if(c->funkcounter >= 128)
		{
			c->funkcounter = 0;
			c->sample->sampledata[c->sample->repeatpoint * 2 + c->funkpos] ^= 0xff;
			c->funkpos = (c->funkpos + 1) % (c->sample->repeatlength * 2);
		}

		for(int i = 0; i < ticktime * rate - 1; i++)
		{
			c->buffer[i] = (float)c->sample->sampledata[c->index++] / 128.0f * c->tempvolume / 64.0 * 0.4f;

			if(c->repeat && (c->index >= (c->sample->repeatlength) * 2 + (c->sample->repeatpoint) * 2))
			{
				c->index = c->sample->repeatpoint * 2;
			}
			else if(c->index >= (c->sample->length) * 2)
			{
				if(c->sample->repeatlength > 1)
				{
					c->index = c->sample->repeatpoint * 2;
					c->repeat = true;
				}
				else
				{
					for(int j = i + 1; j < ticktime * rate - 1; j++)
						c->buffer[j] = 0;
					c->stop = true;
					break;
				}
			}
		}
	}

	src_process(c->converter, c->cdata);

	if(c->cdata->output_frames_gen != c->cdata->output_frames)
	{
		for(int k = c->cdata->output_frames_gen; k < c->cdata->output_frames; k++)
		{
			c->resampled[k] = c->resampled[c->cdata->output_frames_gen - 1];
		}
	}

	// WRITE TO MIXING BUFFER
	if(overwrite)
	{
		for(int i = 0; i < writesize; i++)
		{
			audiobuf[i * 2 + offset] = c->resampled[i];
		}
	}
	else
	{
		for(int i = 0; i < writesize; i++)
		{
			audiobuf[i * 2 + offset] += c->resampled[i];
		}
	}

	if(globaltick == gm->speed - 1)
	{
		c->tempperiod = c->period;
		c->deltick = 0;
	}
}

void sampleparse(modfile* m, uint8_t* filearr, uint32_t start)
{
	for(int i = 0; i < numsamples; i++)
	{
		sample* s = malloc(sizeof(sample));
		m->samples[i] = s;

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

	if(pattern >= gm->songlength)
	{
		if(loop)
		{
			pattern      =    0;
			row          =    0;
			globaltick   =    0;
			gm->speed    =    6;
			nextspeed    =    6;
			gm->tempo    =  125;
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
			renderpattern(gm->patterns + 1024 * gm->patternlist[pattern]);

		patternset = false;
		curdata    = gm->patterns + ((gm->patternlist[pattern]) * 1024) + (16 * row);
		currow     = row;
		curpattern = pattern;

		preprocesseffects(curdata +  0);
		preprocesseffects(curdata +  4);
		preprocesseffects(curdata +  8);
		preprocesseffects(curdata + 12);

		gm->speed = nextspeed;
		gm->tempo = nexttempo;
		ticktime = nextticktime;
	}

	processnote(&cp[0], curdata +  0, 0,  true);
	processnote(&cp[1], curdata +  4, 1,  true);
	processnote(&cp[2], curdata +  8, 1, false);
	processnote(&cp[3], curdata + 12, 0, false);

	globaltick++;
	if(globaltick == gm->speed)
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

modfile* modparse(FILE* f)
{
	uint8_t* filearr = malloc(filelength * sizeof(uint8_t));
	int seek = 0;
	int c;
	
	while((c = fgetc(f)) != EOF)
		filearr[seek++] = (uint8_t)c;

	modfile* m = malloc(sizeof(modfile));
	
	strncpy((char*)m->name, (char*)filearr, 20);
	m->name[20] = '\x00';

	memcpy(m->magicstring, filearr + 1080, 4);
	m->magicstring[4] = '\x00';
	if(strcmp(m->magicstring, "M.K.") && strcmp(m->magicstring, "4CHN"))
		type = 1;	// Not a 31 instrument 4 channel MOD file. May not be playable
	else
		type = 0;

	numsamples = type ? 15 : 31;

	if (type == 0)
		m->songlength = filearr[950];
	else
		m->songlength = filearr[470];

	if (type == 0)
		memcpy(m->patternlist, filearr + 952, 128);
	else
		memcpy(m->patternlist, filearr + 472, 128);

	int max = 0;
	for(int i = 0; i < 128; i++)
	{
		if(m->patternlist[i] > max)
			max = m->patternlist[i];
	}

	uint32_t len = (uint32_t)(1024 * (max + 1)); // 1024 = size of pattern
	m->patterns = malloc(len);

	uint16_t size;
	if(type == 0)
		size = 1084;
	else
		size = 600;

	memcpy(m->patterns, filearr + size, len);
	
	sampleparse(m, filearr, len + size);
	
	m->speed     =    6; // default speed = 6
	nextspeed    =    6;
	m->tempo     =  125;
	nexttempo    =  125;
	ticktime     = 0.02;
	nextticktime = 0.02;

	free(filearr);

	return m;
}
