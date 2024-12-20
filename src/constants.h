#ifndef __CONSTANTS_H
#define __CONSTANTS_H

#define SCREEN					0xa000
#define RRBSCREENWIDTH			80
#define RRBSCREENWIDTH2			(2*RRBSCREENWIDTH)
#define PALETTE					0xc000
#define DIRENTPTRS              0xc300

#define FONTCHARMEM				0x10000

#define COLOR_RAM				0xff80000
#define COLOR_RAM_OFFSET		0x0800
#define SAFE_COLOR_RAM			(COLOR_RAM + COLOR_RAM_OFFSET)
#define SAFE_COLOR_RAM_IN1MB	(SAFE_COLOR_RAM - $ff00000)	

#define MAPPEDCOLOURMEM			0x08000

#define FONTCHARSIZE            0x3200
#define SAMPLEADRESS			(0x10000 + FONTCHARSIZE) // $60000-$13200 = $4ce00 bytes for mod = #314880 bytes = 307kb

// attic mem = 8MB = 0x800000

#define ATTICADDRESS			0x08000000
#define ATTICFONTCHARMEM        (ATTICADDRESS + 0x100000)
#define ATTICDIRENTRIES         (ATTICADDRESS + 0x200000)

#endif
