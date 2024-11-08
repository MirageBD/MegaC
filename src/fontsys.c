#include <stdint.h>
#include "constants.h"
#include "fontsys.h"
#include "macros.h"
#include "dmajobs.h"

uint8_t fontsys_asciiremap[] =
{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, // .repeat $21, i	.charmap $20 + i, $00 + i
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	0x20, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, // .repeat $05, i	.charmap $5b + i, $3b + i
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, // .repeat $20, i	.charmap $60 + i, $20 + i
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
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

void fontsys_map()
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
}

void fontsys_unmap()
{
	UNMAP_ALL
}

void fontsys_clearscreen()
{
	run_dma_job((__far char *)&dma_clearcolorram1);
	run_dma_job((__far char *)&dma_clearcolorram2);
	run_dma_job((__far char *)&dma_clearscreen1);
	run_dma_job((__far char *)&dma_clearscreen2);
}

void fontsys_test()
{
	fontsys_map();

/*
		fnts_row = 2 * 0;
		fnts_column = 0;
		poke(((uint8_t *)&fnts_curpal + 1), 0x4f);
		poke(((uint8_t *)&fnts_readchar + 1), (uint8_t)(((0x6000 + 0*0x0057) >> 0) & 0xff));
		poke(((uint8_t *)&fnts_readchar + 2), (uint8_t)(((0x6000 + 0*0x0057) >> 8) & 0xff));
		fontsys_asm_test();

		fnts_row = 2 * 1;
		fnts_column = 0;
		poke(((uint8_t *)&fnts_curpal + 1), 0x4f);
		poke(((uint8_t *)&fnts_readchar + 1), (uint8_t)(((0x6000 + 1*0x0057) >> 0) & 0xff));
		poke(((uint8_t *)&fnts_readchar + 2), (uint8_t)(((0x6000 + 1*0x0057) >> 8) & 0xff));
		fontsys_asm_test();
*/

	for(uint16_t row = 0; row < 25; row++)
	{
		fnts_row = 2 * row;
		fnts_column = 0;
		poke(((uint8_t *)&fnts_curpal + 1), 0x4f);
		poke(((uint8_t *)&fnts_readchar + 1), (uint8_t)(((0x6000 + row*0x0057) >> 0) & 0xff));
		poke(((uint8_t *)&fnts_readchar + 2), (uint8_t)(((0x6000 + row*0x0057) >> 8) & 0xff));
		fontsys_asm_test();
	}

	fontsys_unmap();
}