#include <calypsi/intrinsics6502.h>

#include "main.h"
#include "chips.h"
#include "macros.h"
#include "constants.h"

#include "iffl.h"
#include "irqload.h"

extern char setborder(char a);
int main()
{
	__disable_interrupts();
	
	CPU.PORTDDR = 65;
	CPU.PORT = 0x35;
	
	VIC4.HOTREG = 0;								// disable hot registers
	
	CIA1.ICR = 0b01111111;							// disable interrupts
	CIA2.ICR = 0b01111111;
	CIA1.ICR;
	CIA2.ICR;
	
	VIC3.ROM8  = 0;									// map I/O (Mega65 memory mapping)
	VIC3.ROMA  = 0;
	VIC3.ROMC  = 0;
	VIC3.CROM9 = 0;
	VIC3.ROME  = 0;

	__asm(" lda #0x00\n"							// unmap any mappings
		  " tax\n"
		  " tay\n"
		  " taz\n"
		  " map\n"
		  " nop");

	VIC3.KEY = 0x47;								// Enable the VIC4
	VIC3.KEY = 0x53;
	
	fl_init();
	fl_waiting();
	
	floppy_iffl_fast_load_init("DATA");
	floppy_iffl_fast_load(); 						// chars
	floppy_iffl_fast_load();						// palette
	floppy_iffl_fast_load();						// song
	
	VIC4.FNRST    = 0;								// disable raster interrupts
	VIC4.FNRSTCMP = 0;

	VIC3.H640 = 1;									// enable 640 horizontal width

	VIC4.CHR16 = 1;									// use wide character lookup (i.e. character data anywhere in memory)
	
	VIC2.MCM = 1;									// set multicolor mode
	
	VIC4.FCLRLO = 1;								// lower block, i.e. 0-255		// use NCM and FCM for all characters
	VIC4.FCLRHI = 1;								// everything above 255

	VIC4.NORRDEL = 0;								// enable rrb double buffering
	
	VIC3.V400    = 1;								// enable 400 vertical height
	VIC4.CHRYSCL = 0;
	VIC4.CHRXSCL = 0x78;

	VIC4.DBLRR = 0;									// disable double-height rrb

	VIC4.SCRNPTR    = (SCREEN & 0xffff);			// set screen pointer
	VIC4.SCRNPTRBNK = (SCREEN & 0xf0000) >> 16;
	VIC4.SCRNPTRMB  = 0x0;

	VIC4.COLPTR = COLOR_RAM_OFFSET;					// set offset to colour ram, so we can use continuous memory

	VIC4.CHRCOUNTMSB = RRBSCREENWIDTH >> 8;			// set RRB screenwidth and linestep
	VIC4.DISPROWS    = RRBSCREENWIDTH;
	VIC4.LINESTEP    = RRBSCREENWIDTH << 1;
	VIC4.CHRCOUNTLSB = RRBSCREENWIDTH;

	set_400();

	// enable audio dma and turn off saturation
	poke(0xd711,0b10000000);
	poke(0xd712,0b00000000);

	VIC2.BORDERCOL = 0;
	VIC2.SCREENCOL = 0;

    setborder(5);

    while(1) {}

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