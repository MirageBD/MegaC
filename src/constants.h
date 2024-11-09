#ifndef __CONSTANTS_H
#define __CONSTANTS_H

#define SCREEN                  0xe000
#define RRBSCREENWIDTH          64
#define RRBSCREENWIDTH2         (2*RRBSCREENWIDTH)
#define PALETTE                 0xc000

#define FONTCHARMEM             0x10000

#define COLOR_RAM		    	0xff80000
#define COLOR_RAM_OFFSET    	0x0800
#define SAFE_COLOR_RAM	    	(COLOR_RAM + COLOR_RAM_OFFSET)
#define SAFE_COLOR_RAM_IN1MB    (SAFE_COLOR_RAM - $ff00000)	

#define MAPPEDCOLOURMEM         0x08000

#define MODADRESS               0x20000

#endif
