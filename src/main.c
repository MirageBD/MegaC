#include <calypsi/intrinsics6502.h>

#include "macros.h"
#include "registers.h"
#include "hyppo.h"
#include "constants.h"
#include "modplay.h"
#include "iffl.h"
#include "irqload.h"
#include "keyboard.h"
#include "sdc.h"

extern void irq_fastload();
extern void irq_main();
extern void sdc_opendir();

void main()
{
	SEI

	CPU.PORT = 0b00110101;									// 0x35 = I/O area visible at $D000-$DFFF, RAM visible at $A000-$BFFF and $E000-$FFFF.

	VIC4.HOTREG = 0;										// disable hot registers

	UNMAP_ALL												// unmap any mappings

	CPU.PORTDDR = 65;										// enable 40Hz

	VIC3.KEY = 0x47;										// Enable the VIC4
	VIC3.KEY = 0x53;										// do I need an eom after this?

	CIA1.ICR = 0b01111111;									// disable interrupts
	CIA2.ICR = 0b01111111;
	CIA1.ICR;
	CIA2.ICR;
	poke(0xd01a,0x00);										// disable IRQ raster interrupts because C65 uses raster interrupts in the ROM

	VIC2.RC = 0x00;											// d012 = 0
	IRQ_VECTORS.IRQ = (volatile uint16_t)&irq_fastload;		// set irq vector

	poke(0xd01a,0x01);										// ACK!

	CLI

	// ------------------------------------------------------------------------------------

	fl_init();
	fl_waiting();
	
	floppy_iffl_fast_load_init("DATA");
	floppy_iffl_fast_load(); 									// chars
	floppy_iffl_fast_load();									// palette
	floppy_iffl_fast_load();									// song

	sdc_setbufferaddressmsb(0x04);								// set SDC buffer address to 0x0800
	sdc_setprocessdirentryptr((uint16_t)(&sdc_processdirentry));	// set dir entry process pointer
	sdc_opendir();

	// ------------------------------------------------------------------------------------

	SEI

	CPU.PORT = 0b00110101;									// 0x35 = I/O area visible at $D000-$DFFF, RAM visible at $A000-$BFFF and $E000-$FFFF.
	
	VIC3.ROM8  = 0;											// map I/O (Mega65 memory mapping)
	VIC3.ROMA  = 0;
	VIC3.ROMC  = 0;
	VIC3.CROM9 = 0;
	VIC3.ROME  = 0;

	VIC4.FNRST    = 0;										// disable raster interrupts
	VIC4.FNRSTCMP = 0;

	VIC3.H640 = 1;											// enable 640 horizontal width

	//VIC4.CHR16 = 1;											// use wide character lookup (i.e. character data anywhere in memory)
	
	//VIC2.MCM = 1;											// set multicolor mode
	
	//VIC4.FCLRLO = 1;										// lower block, i.e. 0-255		// use NCM and FCM for all characters
	//VIC4.FCLRHI = 1;										// everything above 255

	//VIC4.NORRDEL = 0;										// enable rrb double buffering
	
	VIC3.V400    = 1;										// enable 400 vertical height
	VIC4.CHRYSCL = 0;
	VIC4.CHRXSCL = 0x78;

	//VIC4.DBLRR = 0;											// disable double-height rrb

	//VIC4.SCRNPTR    = (SCREEN & 0xffff);					// set screen pointer
	//VIC4.SCRNPTRBNK = (SCREEN & 0xf0000) >> 16;
	//VIC4.SCRNPTRMB  = 0x0;

	//VIC4.COLPTR = COLOR_RAM_OFFSET;							// set offset to colour ram, so we can use continuous memory

	//VIC4.CHRCOUNTMSB = RRBSCREENWIDTH >> 8;					// set RRB screenwidth and linestep
	//VIC4.DISPROWS    = RRBSCREENWIDTH;
	//VIC4.LINESTEP    = RRBSCREENWIDTH << 1;
	//VIC4.CHRCOUNTLSB = RRBSCREENWIDTH;

	/*
	for(uint8_t i=0; i<255; i++)
	{
		poke(0xd100+i, ((uint8_t *)PALETTE)[0*256+i]);
		poke(0xd200+i, ((uint8_t *)PALETTE)[1*256+i]);
		poke(0xd300+i, ((uint8_t *)PALETTE)[2*256+i]);
	}
	*/

	// enable audio dma and turn off saturation
	AUDIO_DMA.AUDEN = 0b10000000;
	AUDIO_DMA.DBGSAT = 0b00000000;

	// Stop all DMA audio first
	AUDIO_DMA.CHANNELS[0].CONTROL = 0;
	AUDIO_DMA.CHANNELS[1].CONTROL = 0;
	AUDIO_DMA.CHANNELS[2].CONTROL = 0;
	AUDIO_DMA.CHANNELS[3].CONTROL = 0;

	modplay_init(0x20000);

	//VIC2.BORDERCOL = 14;
	//VIC2.SCREENCOL = 14;

	CIA1.ICR = 0b01111111;									// disable interrupts
	CIA2.ICR = 0b01111111;
	CIA1.ICR;
	CIA2.ICR;
	
	poke(0xd01a,0x00);										// disable IRQ raster interrupts because C65 uses raster interrupts in the ROM

	VIC2.RC = 0x40;											// d012 = 8
	IRQ_VECTORS.IRQ = (volatile uint16_t)&irq_main;

	poke(0xd01a,0x01);										// ACK!

	CLI

	// ------------------------------------------------------------------------------------

	while(1)
	{
		__asm
		(
			" lda 0xd020\n"
			" sta 0xd020\n"
		);
	}
}