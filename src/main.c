#include "macros.h"
#include "registers.h"
#include "constants.h"
#include "dmajobs.h"
#include "program.h"

extern void irq_fastload();
extern void irq_main();

void main()
{
	SEI

	__asm(" dec a");

	CPU.PORT = 0b00110101;										// 0x35 = I/O area visible at $D000-$DFFF, RAM visible at $A000-$BFFF and $E000-$FFFF.
	VIC4.HOTREG = 0;											// disable hot registers
	UNMAP_ALL													// unmap any mappings
	CPU.PORTDDR = 65;											// enable 40Hz
	VIC3.KEY = 0x47;											// Enable the VIC4
	VIC3.KEY = 0x53;											// do I need an eom after this?
	EOM

	dma_init();

	CIA1.ICR = 0b01111111;										// disable interrupts
	CIA2.ICR = 0b01111111;
	CIA1.ICR;
	CIA2.ICR;
	poke(0xd01a,0x00);											// disable IRQ raster interrupts because C65 uses raster interrupts in the ROM
	VIC2.RC = 0x08;												// d012 = 0
	VIC2.RC8 = 0X00;											// d011
	IRQ_VECTORS.IRQ = (volatile uint16_t)&irq_fastload;			// set irq vector
	poke(0xd01a,0x01);											// ACK!

	CLI

	program_loaddata();

	SEI

	CPU.PORT			= 0b00110101;							// 0x35 = I/O area visible at $D000-$DFFF, RAM visible at $A000-$BFFF and $E000-$FFFF.
	
	VIC3.ROM8			= 0;									// map I/O (Mega65 memory mapping)
	VIC3.ROMA			= 0;
	VIC3.ROMC			= 0;
	VIC3.CROM9			= 0;
	VIC3.ROME			= 0;

	VIC4.FNRST			= 0;									// disable raster interrupts
	VIC4.FNRSTCMP		= 0;
	VIC4.CHR16			= 1;									// use wide character lookup (i.e. character data anywhere in memory)

	VIC4.TEXTXPOSLSB	= 0x90;									// set TEXTXPOS to same as SDBDRWDLSB
	
	VIC2.MCM			= 1;									// set multicolor mode
	VIC4.FCLRLO			= 1;									// lower block, i.e. 0-255		// use NCM and FCM for all characters
	VIC4.FCLRHI			= 1;									// everything above 255
	VIC4.NORRDEL		= 0;									// enable rrb double buffering
	VIC4.DBLRR			= 0;									// disable double-height rrb
	
	VIC3.H640			= 1;									// enable 640 horizontal width
	VIC3.V400			= 1;									// enable 400 vertical height
	VIC4.CHRYSCL		= 0;
	VIC4.CHRXSCL		= 0x78;

	VIC4.DISPROWS		= 50;									// display 50 rows of text

	VIC4.SCRNPTR		= (SCREEN & 0xffff);					// set screen pointer
	VIC4.SCRNPTRBNK		= (SCREEN & 0xf0000) >> 16;
	VIC4.SCRNPTRMB		= 0;

	VIC4.CHRCOUNTLSB	= RRBSCREENWIDTH;						// RRBSCREENWIDTH; // 64 works, 65 does not
	VIC4.CHRCOUNTMSB	= RRBSCREENWIDTH >> 8;					// set RRB screenwidth and linestep
	VIC4.LINESTEP		= RRBSCREENWIDTH2;

	VIC4.COLPTR			= COLOR_RAM_OFFSET;						// set offset to colour ram, so we can use continuous memory

	program_init();

	CIA1.ICR = 0b01111111;										// disable interrupts
	CIA2.ICR = 0b01111111;
	CIA1.ICR;
	CIA2.ICR;
	poke(0xd01a,0x00);											// disable IRQ raster interrupts because C65 uses raster interrupts in the ROM
	VIC2.RC = 0x20;												// d012 = 8
	VIC2.RC8 = 0X00;											// d011
	IRQ_VECTORS.IRQ = (volatile uint16_t)&irq_main;
	poke(0xd01a,0x01);											// ACK!

	CLI

	program_mainloop();
}
