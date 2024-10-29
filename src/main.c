#include <calypsi/intrinsics6502.h>

#include "main.h"
#include "registers.h"
#include "macros.h"
#include "constants.h"
#include "modplay.h"

#include "iffl.h"
#include "irqload.h"

extern char setborder(char a);
extern void irq1();
extern void fastload_irq();

int main()
{

	// ------------------------------------------------------------------------------------

	__disable_interrupts();									// sei

	CPU.PORT = 0b00110101;									// 0x35 = I/O area visible at $D000-$DFFF, RAM visible at $A000-$BFFF and $E000-$FFFF.

	VIC4.HOTREG = 0;										// disable hot registers

	__asm(" lda #0x00\n"									// unmap any mappings
		  " tax\n"
		  " tay\n"
		  " taz\n"
		  " map\n"
		  " nop");

	CPU.PORTDDR = 65;										// enable 40Hz

	VIC3.KEY = 0x47;										// Enable the VIC4
	VIC3.KEY = 0x53;										// do I need an eom after this?

	CIA1.ICR = 0b01111111;									// disable interrupts
	CIA2.ICR = 0b01111111;
	CIA1.ICR;
	CIA2.ICR;
	poke(0xd01a,0x00);										// disable IRQ raster interrupts because C65 uses raster interrupts in the ROM

	VIC2.RC = 0x00;											// d012 = 0
	IRQ_VECTORS.IRQ = (volatile uint16_t)&fastload_irq;		// set irq vector

	poke(0xd01a,0x01);										// ACK!

	__enable_interrupts();									// cli

	// ------------------------------------------------------------------------------------

	fl_init();
	fl_waiting();
	
	floppy_iffl_fast_load_init("DATA");
	floppy_iffl_fast_load(); 								// chars
	floppy_iffl_fast_load();								// palette
	floppy_iffl_fast_load();								// song

	// ------------------------------------------------------------------------------------

	__disable_interrupts();									// sei

	CPU.PORT = 0b00110101;									// 0x35 = I/O area visible at $D000-$DFFF, RAM visible at $A000-$BFFF and $E000-$FFFF.
	
	VIC3.ROM8  = 0;											// map I/O (Mega65 memory mapping)
	VIC3.ROMA  = 0;
	VIC3.ROMC  = 0;
	VIC3.CROM9 = 0;
	VIC3.ROME  = 0;

	VIC4.FNRST    = 0;										// disable raster interrupts
	VIC4.FNRSTCMP = 0;

	VIC3.H640 = 1;											// enable 640 horizontal width

	VIC4.CHR16 = 1;											// use wide character lookup (i.e. character data anywhere in memory)
	
	VIC2.MCM = 1;											// set multicolor mode
	
	VIC4.FCLRLO = 1;										// lower block, i.e. 0-255		// use NCM and FCM for all characters
	VIC4.FCLRHI = 1;										// everything above 255

	VIC4.NORRDEL = 0;										// enable rrb double buffering
	
	VIC3.V400    = 1;										// enable 400 vertical height
	VIC4.CHRYSCL = 0;
	VIC4.CHRXSCL = 0x78;

	VIC4.DBLRR = 0;											// disable double-height rrb

	VIC4.SCRNPTR    = (SCREEN & 0xffff);					// set screen pointer
	VIC4.SCRNPTRBNK = (SCREEN & 0xf0000) >> 16;
	VIC4.SCRNPTRMB  = 0x0;

	VIC4.COLPTR = COLOR_RAM_OFFSET;							// set offset to colour ram, so we can use continuous memory

	VIC4.CHRCOUNTMSB = RRBSCREENWIDTH >> 8;					// set RRB screenwidth and linestep
	VIC4.DISPROWS    = RRBSCREENWIDTH;
	VIC4.LINESTEP    = RRBSCREENWIDTH << 1;
	VIC4.CHRCOUNTLSB = RRBSCREENWIDTH;

	for(uint8_t i=0; i<255; i++)
	{
		poke(0xd100+i, ((uint8_t *)PALETTE)[0*256+i]);
		poke(0xd200+i, ((uint8_t *)PALETTE)[1*256+i]);
		poke(0xd300+i, ((uint8_t *)PALETTE)[2*256+i]);
	}

	set_400();

	// enable audio dma and turn off saturation
	poke(0xd711,0b10000000);
	poke(0xd712,0b00000000);

   // setborder(5);

	// Stop all DMA audio first
	poke(0xd720, 0);
	poke(0xd730, 0);
	poke(0xd740, 0);
	poke(0xd750, 0);

	modplay_init(0x40000);

	VIC2.BORDERCOL = 14;
	VIC2.SCREENCOL = 14;

	CIA1.ICR = 0b01111111;									// disable interrupts
	CIA2.ICR = 0b01111111;
	CIA1.ICR;
	CIA2.ICR;
	
	poke(0xd01a,0x00);										// disable IRQ raster interrupts because C65 uses raster interrupts in the ROM

	VIC2.RC = 0x08;											// d012 = 8
	IRQ_VECTORS.IRQ = (volatile uint16_t)&irq1;				// A9 71 8D FE FF A9 20 8D FF FF

	poke(0xd01a,0x01);										// ACK!

	__enable_interrupts();									// cli

	// ------------------------------------------------------------------------------------

    while(1) {}												//  bra *
    return 0;
}

void set_400()
{
	if (VIC4.PALNTSC)
	{
		VIC4.TBDRPOSLSB  = 0x37;
		VIC4.TBDRPOSMSB  = 0;
		VIC4.BBDRPOSLSB  = 0xc7;
		VIC4.BBDRPOSMSB  = 0x1;
		VIC4.TEXTYPOSLSB = 0x37;
		VIC4.TEXTYPOSMSB = 0;
	}
	else
	{
		VIC4.TBDRPOSLSB  = 0x68;
		VIC4.TBDRPOSMSB  = 0;
		VIC4.BBDRPOSLSB  = 0xf8;
		VIC4.BBDRPOSMSB  = 0x1;
		VIC4.TEXTYPOSLSB = 0x68;
		VIC4.TEXTYPOSMSB = 0;
	}
}