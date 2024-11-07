#ifndef __CONSTANTS_H
#define __CONSTANTS_H

#define SCREEN				0xe000
#define RRBSCREENWIDTH		80
#define RRBSCREENWIDTH2		(2*80)
#define PALETTE             0xc000

#define FONTCHARMEM         0x10000
#define FNTS_NUMCHARS		(1600/16)   // 100 chars, so should only have to bother setting lower value in screenram

#define COLOR_RAM			0xff80000
#define COLOR_RAM_OFFSET	0x0800
#define SAFE_COLOR_RAM		(COLOR_RAM + COLOR_RAM_OFFSET)

#endif
