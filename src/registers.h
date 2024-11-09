// See end of file to see where all the structs are pointed at a register.

#ifndef __CHIPS_H
#define __CHIPS_H

#include <stdint.h>

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------

typedef struct _CPU					// 0x00
{
	uint8_t		PORTDDR;			// 0x00
	uint8_t		PORT;				// 0x01
} _CPU;

// ---------------------------------------------------------------------------

typedef struct _VIC2				// 0xd000
{
	uint8_t		S0X;				// 0xd000
	uint8_t		S0Y;				// 0xd001
	uint8_t		S1X;				// 0xd002
	uint8_t		S1Y;				// 0xd003
	uint8_t		S2X;				// 0xd004
	uint8_t		S2Y;				// 0xd005
	uint8_t		S3X;				// 0xd006
	uint8_t		S3Y;				// 0xd007
	uint8_t		S4X;				// 0xd008
	uint8_t		S4Y;				// 0xd009
	uint8_t		S5X;				// 0xd00a
	uint8_t		S5Y;				// 0xd00b
	uint8_t		S6X;				// 0xd00c
	uint8_t		S6Y;				// 0xd00d
	uint8_t		S7X;				// 0xd00e
	uint8_t		S7Y;				// 0xd00f
	uint8_t		SXMSB;				// 0xd010
	union							// 0xd011
	{
		uint8_t	YSCL : 3;
		uint8_t	RSEL : 1;
		uint8_t	DEN  : 1;
		uint8_t	BMM  : 1;
		uint8_t	ECM  : 1;
		uint8_t	RC8  : 1;
	};
	uint8_t		RC;					// 0xd012
	uint8_t		LPX;				// 0xd013
	uint8_t		LPY;				// 0xd014
	uint8_t		SE;					// 0xd015
	union							// 0xd016
	{
		uint8_t	XSCL : 3;
		uint8_t	CSEL : 1;
		uint8_t	MCM  : 1;
		uint8_t	RST  : 1;
	};
	uint8_t		SEXY;				// 0xd017
	union							// 0xd018
	{
		uint8_t	_NA0 : 1;
		uint8_t	CB   : 3;
		uint8_t	VS   : 4;
	};
	union							// 0xd019
	{
		uint8_t	RIRQ : 1;
		uint8_t	ISBC : 1;
		uint8_t	ISSC : 1;
		uint8_t	ILP  : 1;
	};
	union							// 0xd01a
	{
		uint8_t	MRIRQ : 1;
		uint8_t	MISBC : 1;
		uint8_t	MISSC : 1;
	};
	uint8_t		BSP;				// 0xd01b
	uint8_t		SCM;				// 0xd01c
	uint8_t		SEXX;				// 0xd01d
	uint8_t		SSC;				// 0xd01e
	uint8_t		SBC;				// 0xd01f
	uint8_t		BORDERCOL;			// 0xd020
	uint8_t		SCREENCOL;			// 0xd021
	uint8_t		MC1;				// 0xd022
	uint8_t		MC2;				// 0xd023
	uint8_t		MC3;				// 0xd024
	uint8_t		SPRMC0;				// 0xd025
	uint8_t		SPRMC1;				// 0xd026
	uint8_t		SPR0COL;			// 0xd027
	uint8_t		SPR1COL;			// 0xd028
	uint8_t		SPR2COL;			// 0xd029
	uint8_t		SPR3COL;			// 0xd02a
	uint8_t		SPR4COL;			// 0xd02b
	uint8_t		SPR5COL;			// 0xd02c
	uint8_t		SPR6COL;			// 0xd02d
	uint8_t		SPR7COL;			// 0xd02e
	uint8_t		_NA1;				// 0xd02f
	uint8_t		C128_FAST;			// 0xd030
} _VIC2;

// ---------------------------------------------------------------------------

typedef struct _VIC3				// 0xd02f
{
	uint8_t		KEY; 				// 0xd02f
	union							// 0xd030
	{
		uint8_t	CRAM2K  : 1;
		uint8_t	EXTSYNC : 1;
		uint8_t	PAL     : 1;
		uint8_t	ROM8    : 1;
		uint8_t	ROMA    : 1;
		uint8_t	ROMC    : 1;
		uint8_t	CROM9   : 1;
		uint8_t	ROME    : 1;
	};
	union							// 0xd031
	{
		uint8_t	INT   : 1;
		uint8_t	MONO  : 1;
		uint8_t	H1280 : 1;
		uint8_t	V400  : 1;
		uint8_t	BPM   : 1;
		uint8_t	ATTR  : 1;
		uint8_t	FAST  : 1;
		uint8_t	H640  : 1;
	};
	union							// 0xd032
	{
		uint8_t	_NA1  : 1;
		uint8_t	B0EVN : 3;
		uint8_t	_NA2  : 1;
		uint8_t	B0ODD : 3;
	};
	union							// 0xd033
	{
		uint8_t	_NA3  : 1;
		uint8_t	B1EVN : 3;
		uint8_t	_NA4  : 1;
		uint8_t	B1ODD : 3;
	};
	union							// 0xd034
	{
		uint8_t	_NA5  : 1;
		uint8_t	B2EVN : 3;
		uint8_t	_NA6  : 1;
		uint8_t	B2ODD : 3;
	};
	union							// 0xd035
	{
		uint8_t	_NA7  : 1;
		uint8_t	B3EVN : 3;
		uint8_t	_NA8  : 1;
		uint8_t	B3ODD : 3;
	};
	union							// 0xd036
	{
		uint8_t	_NA9  : 1;
		uint8_t	B4EVN : 3;
		uint8_t	_NA10 : 1;
		uint8_t	B4ODD : 3;
	};
	union							// 0xd037
	{
		uint8_t	_NA11 : 1;
		uint8_t	B5EVN : 3;
		uint8_t	_NA12 : 1;
		uint8_t	B5ODD : 3;
	};
	union							// 0xd038
	{
		uint8_t	_NA13 : 1;
		uint8_t	B6EVN : 3;
		uint8_t	_NA14 : 1;
		uint8_t	B6ODD : 3;
	};
	union							// 0xd039
	{
		uint8_t	_NA15 : 1;
		uint8_t	B7EVN : 3;
		uint8_t	_NA16 : 1;
		uint8_t	B7ODD : 3;
	};
	uint8_t		BPCOMP;				// 0xd03a
	uint8_t		BPX;				// 0xd03b
	uint8_t		BPY;				// 0xd03c
	uint8_t		HPOS;				// 0xd03d
	uint8_t		VPOS;				// 0xd03e
	uint8_t		B0PIX;				// 0xd03f
	uint8_t		B1PIX;				// 0xd040
	uint8_t		B2PIX;				// 0xd041
	uint8_t		B3PIX;				// 0xd042
	uint8_t		B4PIX;				// 0xd043
	uint8_t		B5PIX;				// 0xd044
	uint8_t		B6PIX;				// 0xd045
	uint8_t		B7PIX;				// 0xd046
	uint8_t		_NA17    [0xB9];	// 0xd047
	uint8_t		PALRED   [0x100];	// 0xd100
	uint8_t		PALGREEN [0x100];	// 0xd200
	uint8_t		PALBLUE  [0x100];	// 0xd300
} _VIC3;

// ---------------------------------------------------------------------------

typedef struct _VIC4				// 0xd048
{
	uint8_t		TBDRPOSLSB;			// 0xd048
	union							// 0xd049
	{
		uint8_t	TBDRPOSMSB  : 4;
		uint8_t	SPRBPMENLSB : 4;
	};
	uint8_t		BBDRPOSLSB;			// 0xd04a
	union							// 0xd04b
	{
		uint8_t	BBDRPOSMSB  : 4;
		uint8_t	SPRBPMENMSB : 4;
	};
	uint8_t		TEXTXPOSLSB;		// 0xd04c
	union							// 0xd04d
	{
		uint8_t	TEXTXPOSMSB : 4;
		uint8_t	SPRTILENLSB : 4;
	};
	uint8_t		TEXTYPOSLSB;		// 0xd04e
	union							// 0xd04f
	{
		uint8_t	TEXTYPOSMSB : 4;
		uint8_t	SPRTILENMSB : 4;
	};
	uint8_t		XPOSLSB;			// 0xd050
	union							// 0xd051
	{
		uint8_t	XPOSMSB : 6;
		uint8_t	DBLRR   : 1;
		uint8_t	NORRDEL : 1;
	};
	uint8_t		FNRASTERLSB;		// 0xd052
	union							// 0xd053
	{
		uint8_t	FNRASTERMSB : 3;
		uint8_t	_NA1        : 2;
		uint8_t	UPSCALE     : 1;
		uint8_t	SHDEMU      : 1;
		uint8_t	FNRST       : 1;
	};
	union							// 0xd054
	{
		uint8_t	CHR16  : 1;
		uint8_t	FCLRLO : 1;
		uint8_t	FCLRHI : 1;
		uint8_t	SMTH   : 1;
		uint8_t	SPR640 : 1;
		uint8_t	PALEMU : 1;
		uint8_t	VFAST  : 1;
		uint8_t	ALPHEN : 1;
	};
	uint8_t		SPRHGTEN;			// 0xd055
	uint8_t		SPRHGHT;			// 0xd056
	uint8_t		SPRX64EN;			// 0xd057
	uint16_t	LINESTEP;			// 0xd058
	uint8_t		CHRXSCL;			// 0xd05a
	uint8_t		CHRYSCL;			// 0xd05b
	uint8_t		SDBDRWDLSB;			// 0xd05c
	union							// 0xd05d
	{
		uint8_t	SDBDRWDMSB : 6;
		uint8_t	RSTDELEN   : 1;
		uint8_t	HOTREG     : 1;
	};
	uint8_t		CHRCOUNTLSB;		// 0xd05e
	uint8_t		SPRXSMSBS;			// 0xd05f
	uint16_t	SCRNPTR;			// 0xd060
	uint8_t		SCRNPTRBNK;			// 0xd062
	union							// 0xd063
	{
		uint8_t	SCRNPTRMB   : 4;
		uint8_t	CHRCOUNTMSB : 2;
		uint8_t	FCOLMCM     : 1;
		uint8_t	EXGLYPH     : 1;
	};
	uint16_t	COLPTR;				// 0xd064
	uint8_t		_NA2;				// 0xd066
	uint8_t		SBPDEBUG;			// 0xd067
	uint16_t	CHARPTR;			// 0xd068
	uint8_t		CHARPTRBNK;			// 0xd06a
	uint8_t		SPR16EN;			// 0xd06b
	uint16_t	SPRPTRADR;			// 0xd06c
	union							// 0xd06e
	{
		uint8_t	SPRPTADRRBNK : 7;
		uint8_t	SPRPTR16     : 1;
	};
	union							// 0xd06f
	{
		uint8_t	RASLINE0 : 6;
		uint8_t	VGAHDTV  : 1;
		uint8_t	PALNTSC  : 1;
	};
	union							// 0xd070
	{
		uint8_t	ABTPALSEL : 2;
		uint8_t	SPRPALSEL : 2;
		uint8_t	BTPALSEL  : 2;
		uint8_t	MAPEDPAL  : 2;
	};
	uint8_t		BP16ENS;			// 0xd071
	uint8_t		SPRYADJ;			// 0xd072
	union							// 0xd073
	{
		uint8_t	ALPHADELAY   : 4;
		uint8_t	RASTERHEIGHT : 4;
	};
	uint8_t		SPRENALPHA;			// 0xd074
	uint8_t		SPRALPHAVAL;		// 0xd075
	uint8_t		SPRENV400;			// 0xd076
	uint8_t		SRPYMSBS;			// 0xd077
	uint8_t		SPRYSMSBS;			// 0xd078
	uint8_t		RSTCOMPLSB;			// 0xd079
	union							// 0xd07a
	{
		uint8_t	RSTCMPMSB   : 3;
		uint8_t	SPTRCONT    : 1;
		uint8_t	_NA3        : 1;
		uint8_t	NOBUGCOMPAT : 1;
		uint8_t	EXTIRQS     : 1;
		uint8_t	FNRSTCMP    : 1;
	};
	uint8_t		DISPROWS;			// 0xd07b
	union							// 0xd07c
	{
		uint8_t	BITPBANK : 3;
		uint8_t	_NA4     : 1;
		uint8_t	HSYNCP   : 1;
		uint8_t	VSYNCP   : 1;
		uint8_t	DEBUGC   : 2;
	};
	uint8_t		_NA5[0x84];			// 0xd07d
	uint8_t		PALRED   [0x100];	// 0xd100
	uint8_t		PALGREEN [0x100];	// 0xd200
	uint8_t		PALBLUE  [0x100];	// 0xd300
} _VIC4;

// ---------------------------------------------------------------------------

typedef struct _FDC					// 0xd080
{
	union							// 0xd080
	{
		uint8_t	DS    : 3;
		uint8_t	SIDEF : 1;
		uint8_t	SWAP  : 1;
		uint8_t	MOTOR : 1;
		uint8_t	LED   : 1;
		uint8_t	IRQ   : 1;
	};
	union							// 0xd081
	{
		uint8_t	NOBUF : 1;
		uint8_t	ALT   : 1;
		uint8_t	ALGO  : 1;
		uint8_t	DIR   : 1;
		uint8_t	STEPF : 1;
		uint8_t	FREE  : 1;
		uint8_t	RDCMD : 1;
		uint8_t	WRCMD : 1;
	};
	union							// 0xd082
	{
		uint8_t	TK0  : 1;
		uint8_t	PROT : 1;
		uint8_t	LOST : 1;
		uint8_t	CRC  : 1;
		uint8_t	RNF  : 1;
		uint8_t	EQ   : 1;
		uint8_t	DRQ  : 1;
		uint8_t	BUSY : 1;
	};
	union							// 0xd083
	{
		uint8_t	DSKCHG  : 1;
		uint8_t	C65_IRQ : 1;
		uint8_t	INDEX   : 1;
		uint8_t	DISKIN  : 1;
		uint8_t	WGATE   : 1;
		uint8_t	RUN     : 1;
		uint8_t	WTREQ   : 1;
		uint8_t	RDREQ   : 1;
	};
	uint8_t		TRACK;				// 0xd084
	uint8_t		SECTOR;				// 0xd085
	uint8_t		SIDE;				// 0xd086
	uint8_t		DATA;				// 0xd087
	uint8_t		CLOCK;				// 0xd088
	uint8_t		STEP;				// 0xd089
	uint8_t		PCODE;				// 0xd08a
} _FDC;

// ---------------------------------------------------------------------------

typedef struct _SID_VOICE
{
	uint16_t	FREQUENCY;
	uint16_t	PULSE_WIDTH;
	uint8_t		CONTROL;
	uint8_t		ATTACKDECAY;
	uint8_t		SUSTAINRELEASE;
} _SID_VOICE;

typedef struct _SID					// SID1=0xd400, SID2=0xd420, SID3=0xd440, SID4=0xd460
{
	_SID_VOICE	VOICE[3];
	uint8_t		CUTOFF_LSB;
	uint8_t		CUTOFF_MSB;
	uint8_t		RESONANCE_ROUTING;
	uint8_t		VOLUME_FTYPE;
} _SID;

// ---------------------------------------------------------------------------

typedef struct _IO					// 0xd609
{
	union							// 0xd609
	{
		uint8_t	UFAST  : 1;
		uint8_t	NA     : 7;
	};
	union							// 0xd60a
	{
		uint8_t	MODKEYLSHFT : 1;
		uint8_t	MODKEYRSHFT : 1;
		uint8_t	MODKEYCTRL  : 1;
		uint8_t	MODKEYMEGA  : 1;
		uint8_t	MODKEYALT   : 1;
		uint8_t	MODKEYSCRL  : 1;
		uint8_t	MODKEYCAPS  : 1;
		uint8_t	KEYQUEUE    : 1;
	};
	union							// 0xd60b
	{
		uint8_t	PORTF  : 6;
		uint8_t	OSKZON : 1;
		uint8_t	OSKZEN : 1;
	};
	union							// 0xd60c
	{
		uint8_t	PORTFDDR1 : 6;
		uint8_t	PORTFDDR2 : 2;
	};
	union							// 0xd60d
	{
		uint8_t	CONN41 : 1;
		uint8_t	RST41  : 1;
		uint8_t	SDDATA : 1;
		uint8_t	SDCLK  : 1;
		uint8_t	SDCS   : 1;
		uint8_t	SDBSH  : 1;
		uint8_t	HDSDA  : 1;
		uint8_t	HDSCL  : 1;
	};
	uint8_t		BASHDDR;			// 0xd60e
	union							// 0xd60f
	{
		uint8_t	KEYLEFT   : 1;
		uint8_t	KEYUP     : 1;
		uint8_t	_NA3      : 3;
		uint8_t	REALHW    : 1;
		uint8_t	OSKDIM    : 1;
		uint8_t	ACCESSKEY : 1;
	};
	uint8_t		ASCIIKEY;			// 0xd610
	union							// 0xd611
	{
		uint8_t	MLSHFT   : 1;
		uint8_t	MRSHFT   : 1;
		uint8_t	MCTRL    : 1;
		uint8_t	MMEGA    : 1;
		uint8_t	MALT     : 1;
		uint8_t	MSCRL    : 1;
		uint8_t	MCAPS    : 1;
		uint8_t	MDISABLE : 1;
	};
	union							// 0xd612
	{
		uint8_t	_NA4     : 4;
		uint8_t	OSKDEBUG : 1;
		uint8_t	JOYSWAP  : 1;
		uint8_t	LJOYA    : 1;
		uint8_t	LJOYB    : 1;
	};
	uint8_t		KEYMATRIXPEEK;		// 0xd613
	uint8_t		KEYMATRIXSEL;		// 0xd614
} _IO;

// ---------------------------------------------------------------------------

typedef struct _DMA					// 0xd700
{
	uint8_t		ADDRLSBTRIG;		// 0xd700
	uint8_t		ADDRMSB;			// 0xd701
	uint8_t		ADDRBANK;			// 0xd702
	uint8_t		EN018B;				// 0xd703
	uint8_t		ADDRMB;				// 0xd704
	uint8_t		ETRIG;				// 0xd705
	uint8_t		ETRIGMAPD;			// 0xd706
	uint8_t		_NA[7];				// 0xd707
	uint8_t		ADDRLSB;			// 0xd70e
} _DMA;

// ---------------------------------------------------------------------------

typedef struct _MATH				// 0xd70f
{
	union							// 0xd70f
	{
		uint8_t	_NA1    : 6;
		uint8_t	MULBUSY : 1;
		uint8_t	DIVBUSY : 1;
	};
	uint8_t		_NA2[88];			// 0xd710
	uint8_t		DIVOUT0;			// 0xd768
	uint8_t		DIVOUT1;			// 0xd769
	uint8_t		DIVOUT2;			// 0xd76a
	uint8_t		DIVOUT3;			// 0xd76b
	uint8_t		DIVOUTFRACT0;		// 0xd76c
	uint8_t		DIVOUTFRACT1;		// 0xd76d
	uint8_t		DIVOUTFRACT2;		// 0xd76e
	uint8_t		DIVOUTFRACT3;		// 0xd76f
	uint8_t		MULTINA0;			// 0xd770
	uint8_t		MULTINA1;			// 0xd771
	uint8_t		MULTINA2;			// 0xd772
	uint8_t		MULTINA3;			// 0xd773
	uint8_t		MULTINB0;			// 0xd774
	uint8_t		MULTINB1;			// 0xd775
	uint8_t		MULTINB2;			// 0xd776
	uint8_t		MULTINB3;			// 0xd777
	uint8_t		MULTOUT0;			// 0xd778
	uint8_t		MULTOUT1;			// 0xd779
	uint8_t		MULTOUT2;			// 0xd77a
	uint8_t		MULTOUT3;			// 0xd77b
} _MATH;

// ---------------------------------------------------------------------------

typedef struct _AUDIO_CHANNEL		// 0xd720
{
	uint8_t		CONTROL;			// 0xd720
	uint16_t	BADDR;				// 0xd721
	uint8_t		BADDRMB;			// 0xd723
	uint16_t	FREQ;				// 0xd724
	uint8_t		FREQMB;				// 0xd726
	uint16_t	TADDR;				// 0xd727
	uint8_t		VOLUME;				// 0xd728
	uint16_t	CURADDR;			// 0xd72a
	uint8_t		CURADDRMB;			// 0xd72c
	uint16_t	TMRADDR;			// 0xd72d
	uint8_t		TMRADDRMB;			// 0xd72f
} _AUDIO_CHANNEL;

typedef struct _AUDIO_DMA			// 0xd711
{
	uint8_t		AUDEN;				// AUDEN BLKD AUDWRBLK NOMIX – AUDBLKTO											0xd711
	uint8_t		DBGSAT;				// DBGLFTSAT DBGRGTSAT DBGPANLFTSAT DBGPANRGTSAT – - CHANSWAP SATURATE			0xd712
	uint8_t		IRQCH0FLG;			// IRQCH1FLG IRQCH2FLG IRQCH3FLG IRQCH0ENA IRQCH1ENA IRQCH2ENA IRQCH3ENA		0xd713
	uint8_t		IRQCH0OFFS;			// 0xd714
	uint8_t		IRQCH1OFFS;			// 0xd715
	uint8_t		IRQCH2OFFS;			// 0xd716
	uint8_t		IRQCH3OFFS;			// 0xd717
	uint8_t		_NA3[4];			// 0xd718
	uint8_t		CH0RVOL;			// 0xd71c
	uint8_t		CH1RVOL;			// 0xd71d
	uint8_t		CH2LVOL;			// 0xd71e
	uint8_t		CH3LVOL;			// 0xd71f
	
	_AUDIO_CHANNEL CHANNELS[4];		// 0xd720, 0xd730, 0xd740, 0xd750
} _AUDIO_DMA;

// ---------------------------------------------------------------------------

typedef struct _CIA1				// 0xdc00
{
	uint8_t		PORTA;				// 0xdc00
	uint8_t		PORTB;				// 0xdc01
	uint8_t		DDRA;				// 0xdc02
	uint8_t		DDRB;				// 0xdc03
	uint16_t	TIMERA;				// 0xdc04
	uint16_t	TIMERB;				// 0xdc06
	uint8_t		TODJIF;				// 0xdc08
	uint8_t		TODSEC;				// 0xdc09
	uint8_t		TODMIN;				// 0xdc0a
	uint8_t		TODHOUR;			// 0xdc0b
	uint8_t		SDR;				// 0xdc0c
	uint8_t		ICR;				// 0xdc0d
	uint8_t		CRA;				// 0xdc0e
	uint8_t		CRB;				// 0xdc0f
	uint8_t		TALATCH0;			// 0xdc10
	uint8_t		TALATCH1;			// 0xdc11
	uint8_t		TALATCH2;			// 0xdc12
	uint8_t		TALATCH3;			// 0xdc13
	uint8_t		TALATCH4;			// 0xdc14
	uint8_t		TALATCH5;			// 0xdc15
	uint8_t		TALATCH6;			// 0xdc16
	uint8_t		TALATCH7;			// 0xdc17
	uint8_t		IRM;				// 0xdc18
	uint8_t		GS_TODSEC;			// 0xdc19
	uint8_t		GS_TODMIN;			// 0xdc1a
	uint8_t		GS_TODHOUR;			// 0xdc1b
	uint8_t		ALRMJIF;			// 0xdc1c
	uint8_t		ALRMSEC;			// 0xdc1d
	uint8_t		ALRMMIN;			// 0xdc1e
	uint8_t		ALRMHOUR;			// 0xdc1f
} _CIA1;

// ---------------------------------------------------------------------------

typedef struct _CIA2				// 0xdd00
{
	uint8_t		PORTA;				// 0xdd00
	uint8_t		PORTB;				// 0xdd01
	uint8_t		DDRA;				// 0xdd02
	uint8_t		DDRB;				// 0xdd03
	uint16_t	TIMERA;				// 0xdd04
	uint16_t	TIMERB;				// 0xdd06
	uint8_t		TODJIF;				// 0xdd08
	uint8_t		TODSEC;				// 0xdd09
	uint8_t		TODMIN;				// 0xdd0a
	uint8_t		TODHOUR;			// 0xdd0b
	uint8_t		SDR;				// 0xdd0c
	uint8_t		ICR;				// 0xdd0d
	uint8_t		CRA;				// 0xdd0e
	uint8_t		CRB;				// 0xdd0f
	uint8_t		TALATCH0;			// 0xdd10
	uint8_t		TALATCH1;			// 0xdd11
	uint8_t		TALATCH2;			// 0xdd12
	uint8_t		TALATCH3;			// 0xdd13
	uint8_t		TALATCH4;			// 0xdd14
	uint8_t		TALATCH5;			// 0xdd15
	uint8_t		TALATCH6;			// 0xdd16
	uint8_t		TALATCH7;			// 0xdd17
	uint8_t		IRM;				// 0xdd18
	uint8_t		GS_TODSEC;			// 0xdd19
	uint8_t		GS_TODMIN;			// 0xdd1a
	uint8_t		GS_TODHOUR;			// 0xdd1b
	uint8_t		ALRMJIF;			// 0xdd1c
	uint8_t		ALRMSEC;			// 0xdd1d
	uint8_t		ALRMMIN;			// 0xdd1e
	uint8_t		ALRMHOUR;			// 0xdd1f
} _CIA2;

// ---------------------------------------------------------------------------

typedef struct _IRQ_VECTORS			// 0xfffa
{
	uint16_t	NMI;				// 0xfffa
	uint16_t	COLDSTART;			// 0xfffc
	uint16_t	IRQ;				// 0xfffe
} _IRQ_VECTORS;

// ---------------------------------------------------------------------------

#define CPU			(*(volatile _CPU *)			  0x00)
#define VIC2		(*(volatile _VIC2 *)		0xd000)
#define VIC3		(*(volatile _VIC3 *)		0xd02f)
#define VIC4		(*(volatile _VIC4 *)		0xd048)
#define FDC			(*(volatile _FDC *)			0xd080)
#define SID1		(*(volatile _SID *)			0xd400)
#define SID2		(*(volatile _SID *)			0xd420)
#define SID3		(*(volatile _SID *)			0xd440)
#define SID4		(*(volatile _SID *)			0xd460)
#define IO			(*(volatile _IO *)			0xd609)
#define DMA			(*(volatile _DMA *)			0xd700)
#define MATH		(*(volatile _MATH *)		0xd70f)
#define AUDIO_DMA	(*(volatile _AUDIO_DMA *)	0xd711)
#define CIA1		(*(volatile _CIA1 *)		0xdc00)
#define CIA2		(*(volatile _CIA2 *)		0xdd00)
#define IRQ_VECTORS	(*(volatile _IRQ_VECTORS *)	0xfffa)

#endif
