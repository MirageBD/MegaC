#include <stdint.h>
#include "constants.h"
#include "fontsys.h"
#include "macros.h"

uint8_t asciiremap[] =
{
    0
};

void fontsys_init()
{
    fontsys_asm_init();

    for(uint8_t row = 0; row < 50; row++)
    {
        ((uint8_t *)&fnts_screentablo)[row] = (uint8_t)(((SCREEN          + RRBSCREENWIDTH2 * row) >> 0) & 0xff);
        ((uint8_t *)&fnts_screentabhi)[row] = (uint8_t)(((SCREEN          + RRBSCREENWIDTH2 * row) >> 8) & 0xff);
        ((uint8_t *)&fnts_attribtablo)[row] = (uint8_t)(((MAPPEDCOLOURMEM + RRBSCREENWIDTH2 * row) >> 0) & 0xff);
        ((uint8_t *)&fnts_attribtabhi)[row] = (uint8_t)(((MAPPEDCOLOURMEM + RRBSCREENWIDTH2 * row) >> 8) & 0xff);
    }
}

void fontsys_test()
{
    
	__asm(  // MAP_MEMORY_IN1MB %0000, %1111
		" lda #0xff\n"
		" ldx #0b00000000\n"                // %0000
		" ldy #0xff\n"
		" ldz #0b00001111\n"
		" map"
	);

	__asm(  //	MAP_MEMORY $00000, %0000, SAFE_COLOR_RAM_IN1MB, %0001
		" lda #0x00\n"          // lda #<((offsetlower32kb) / 256)
		" ldx #0b00000000\n"    // ldx #>((offsetlower32kb) / 256) | (enablemasklower32kb << 4)
		" ldy #0x88\n"          // #<((offsetupper32kb - $8000) / 256)
		" ldz #0x17\n"          // #>((offsetupper32kb - $8000) / 256) | (enablemaskupper32kb << 4)
		" map\n"
		" eom\n"
	);

    fontsys_asm_test();

    UNMAP_ALL
}