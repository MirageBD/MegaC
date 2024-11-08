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
#include "dma.h"
#include "fontsys.h"

extern void irq_fastload();
extern void irq_main();
extern void sdc_opendir();

dma_job dma_clearcolorram1 =
{
	.type					= 0x0a,
	.sbank_token			= 0x80,
	.sbank					= 0x00,
	.dbank_token			= 0x81,
	.dskipratefrac_token	= 0x84,
	.dskipratefrac			= 0x00,
	.dskiprate_token		= 0x85,
	.dskiprate				= 0x02,
	.dbank					= ((SAFE_COLOR_RAM) >> 20),
	.end_options			= 0x00,
	.command				= 0b00000011, // fill, no chain
	.count					= (RRBSCREENWIDTH*50),
	.source					= 0b0000000000001000, // 00001000 = NCM chars
	.source_bank			= 0x00,
	.destination			= ((SAFE_COLOR_RAM) & 0xffff),
	.destination_bank		= (((SAFE_COLOR_RAM) >> 16) & 0x0f),
	.modulo					= 0x0000
};

dma_job dma_clearcolorram2 =
{
	.type					= 0x0a,
	.sbank_token			= 0x80,
	.sbank					= 0x00,
	.dbank_token			= 0x81,
	.dskipratefrac_token	= 0x84,
	.dskipratefrac			= 0x00,
	.dskiprate_token		= 0x85,
	.dskiprate				= 0x02,
	.dbank					= ((SAFE_COLOR_RAM + 1) >> 20),
	.end_options			= 0x00,
	.command				= 0b00000011, // fill, no chain
	.count					= (RRBSCREENWIDTH*50),
	.source					= 0b0000000000001111, // 00001111 = $0f = pixels with value $0f take on the colour value of $0f as well
	.source_bank			= 0x00,
	.destination			= ((SAFE_COLOR_RAM + 1) & 0xffff),
	.destination_bank		= (((SAFE_COLOR_RAM + 1) >> 16) & 0x0f),
	.modulo					= 0x0000
};

dma_job dma_clearscreen1 =
{
	.type					= 0x0a,
	.sbank_token			= 0x80,
	.sbank					= 0x00,
	.dbank_token			= 0x81,
	.dskipratefrac_token	= 0x84,
	.dskipratefrac			= 0x00,
	.dskiprate_token		= 0x85,
	.dskiprate				= 0x02,
	.dbank					= ((SCREEN) >> 20),
	.end_options			= 0x00,
	.command				= 0b00000011, // fill, no chain
	.count					= (RRBSCREENWIDTH*50),
	.source					= (((FONTCHARMEM/64 + 0 /* star=10 */) >> 0)) & 0xff,
	.source_bank			= 0x00,
	.destination			= ((SCREEN) & 0xffff),
	.destination_bank		= (((SCREEN) >> 16) & 0x0f),
	.modulo					= 0x0000
};

dma_job dma_clearscreen2 =
{
	.type					= 0x0a,
	.sbank_token			= 0x80,
	.sbank					= 0x00,
	.dbank_token			= 0x81,
	.dskipratefrac_token	= 0x84,
	.dskipratefrac			= 0x00,
	.dskiprate_token		= 0x85,
	.dskiprate				= 0x02,
	.dbank					= ((SCREEN + 1) >> 20),
	.end_options			= 0x00,
	.command				= 0b00000011, // fill, no chain
	.count					= (RRBSCREENWIDTH*50),
	.source					= (((FONTCHARMEM/64 + 0 /* star=10 */) >> 8)) & 0xff,
	.source_bank			= 0x00,
	.destination			= ((SCREEN + 1) & 0xffff),
	.destination_bank		= (((SCREEN + 1) >> 16) & 0x0f),
	.modulo					= 0x0000
};

void setup_fastload_irq()
{
	CPU.PORT = 0b00110101;										// 0x35 = I/O area visible at $D000-$DFFF, RAM visible at $A000-$BFFF and $E000-$FFFF.

	VIC4.HOTREG = 0;											// disable hot registers

	UNMAP_ALL													// unmap any mappings

	CPU.PORTDDR = 65;											// enable 40Hz

	VIC3.KEY = 0x47;											// Enable the VIC4
	VIC3.KEY = 0x53;											// do I need an eom after this?

	CIA1.ICR = 0b01111111;										// disable interrupts
	CIA2.ICR = 0b01111111;
	CIA1.ICR;
	CIA2.ICR;
	poke(0xd01a,0x00);											// disable IRQ raster interrupts because C65 uses raster interrupts in the ROM

	VIC2.RC = 0x00;												// d012 = 0
	IRQ_VECTORS.IRQ = (volatile uint16_t)&irq_fastload;			// set irq vector

	poke(0xd01a,0x01);											// ACK!
}

/*
	FAT dir entry structure:

	Offset	Type		Description
	--------------------------------------------------------
	$00		asciiz		The long file name
	$40		byte		The length of long file name
	$41		ascii		The ”8.3” file name.
						The name part is padded with spaces to make it exactly 8 bytes. The 3 bytes of the extension follow. There is no . between the name and the extension. There is no NULL byte.
	$4e		dword		The cluster number where the file begins. For sub-directories, this is where the FAT dir entries start for that sub-directory.
	$52		dword		The length of file in bytes.
	$56		byte		The type and attribute bits.
	
						Attribute Bit		bit set
						0					Read only
						1					Hidden
						2					System
						3					Volume label
						4					Sub-directory
						5					Archive
						6					Undefined
						7					Undefined
*/

#define DIR_ENTRY_SIZE	0x57

uint8_t*	direntryptr		= (uint8_t *)0x6000;
uint16_t	numdirentries	= 0;

void main_processdirentry()
{
	uint8_t* transbuf = (uint8_t *)(sdc_transferbuffermsb * 256);

	for(uint16_t i = 0; i < DIR_ENTRY_SIZE; i++)
		direntryptr[i] = fontsys_asciiremap[transbuf[i]];

	uint8_t attribute = transbuf[0x56];
	uint8_t isdir = ((attribute & 0b00010000) == 0b00010000);

	direntryptr += DIR_ENTRY_SIZE;
	numdirentries++;
}

void printdir()
{

}

void setup_main()
{
	CPU.PORT = 0b00110101;										// 0x35 = I/O area visible at $D000-$DFFF, RAM visible at $A000-$BFFF and $E000-$FFFF.
	
	VIC3.ROM8  = 0;												// map I/O (Mega65 memory mapping)
	VIC3.ROMA  = 0;
	VIC3.ROMC  = 0;
	VIC3.CROM9 = 0;
	VIC3.ROME  = 0;

	VIC4.FNRST    = 0;											// disable raster interrupts
	VIC4.FNRSTCMP = 0;

	VIC4.TEXTXPOSLSB = 0x50;									// set TEXTXPOS to same as SDBDRWDLSB

	VIC3.H640 = 1;												// enable 640 horizontal width

	VIC4.CHR16 = 1;												// use wide character lookup (i.e. character data anywhere in memory)
	
	// VIC2.MCM = 1;												// set multicolor mode
	
	//VIC4.FCLRLO = 1;											// lower block, i.e. 0-255		// use NCM and FCM for all characters
	//VIC4.FCLRHI = 1;											// everything above 255

	VIC4.NORRDEL = 0;											// enable rrb double buffering
	
	VIC3.V400    = 1;											// enable 400 vertical height
	VIC4.CHRYSCL = 0;
	VIC4.CHRXSCL = 0x78;

	VIC4.DBLRR = 0;												// disable double-height rrb

	VIC4.DISPROWS = 50;											// display 50 rows of text

	VIC4.SCRNPTR    = (SCREEN & 0xffff);						// set screen pointer
	VIC4.SCRNPTRBNK = (SCREEN & 0xf0000) >> 16;
	VIC4.SCRNPTRMB  = 0x0;

	VIC4.COLPTR = COLOR_RAM_OFFSET;								// set offset to colour ram, so we can use continuous memory

	run_dma_job((__far char *)&dma_clearcolorram1);
	run_dma_job((__far char *)&dma_clearcolorram2);

	run_dma_job((__far char *)&dma_clearscreen1);
	run_dma_job((__far char *)&dma_clearscreen2);

	// poke(0xe000 + (0 * RRBSCREENWIDTH2) + 0, 4 + (0 * FNTS_NUMCHARS));
	// poke(0xe000 + (1 * RRBSCREENWIDTH2) + 0, 4 + (1 * FNTS_NUMCHARS));

	// poke(0xe000 + (0 * RRBSCREENWIDTH2) + 2, 5 + (0 * FNTS_NUMCHARS));
	// poke(0xe000 + (1 * RRBSCREENWIDTH2) + 2, 5 + (1 * FNTS_NUMCHARS));

	VIC4.CHRCOUNTMSB = RRBSCREENWIDTH >> 8;						// set RRB screenwidth and linestep
	VIC4.DISPROWS    = RRBSCREENWIDTH;
	VIC4.LINESTEP    = RRBSCREENWIDTH << 1;
	VIC4.CHRCOUNTLSB = RRBSCREENWIDTH;

	for(uint8_t i=0; i<255; i++)
	{
		poke(0xd100+i, ((uint8_t *)PALETTE)[0*256+i]);
		poke(0xd200+i, ((uint8_t *)PALETTE)[1*256+i]);
		poke(0xd300+i, ((uint8_t *)PALETTE)[2*256+i]);
	}

	// enable audio dma and turn off saturation
	AUDIO_DMA.AUDEN = 0b10000000;
	AUDIO_DMA.DBGSAT = 0b00000000;

	// Stop all DMA audio first
	AUDIO_DMA.CHANNELS[0].CONTROL = 0;
	AUDIO_DMA.CHANNELS[1].CONTROL = 0;
	AUDIO_DMA.CHANNELS[2].CONTROL = 0;
	AUDIO_DMA.CHANNELS[3].CONTROL = 0;

	modplay_init(0x20000);

	VIC2.BORDERCOL = 0x0f;
	VIC2.SCREENCOL = 0x00;

	CIA1.ICR = 0b01111111;										// disable interrupts
	CIA2.ICR = 0b01111111;
	CIA1.ICR;
	CIA2.ICR;
	
	poke(0xd01a,0x00);											// disable IRQ raster interrupts because C65 uses raster interrupts in the ROM

	VIC2.RC = 0xf0;												// d012 = 8
	IRQ_VECTORS.IRQ = (volatile uint16_t)&irq_main;

	poke(0xd01a,0x01);											// ACK!
}

void main()
{
	SEI
	setup_fastload_irq();
	CLI

	fl_init();
	fl_waiting();
	
	floppy_iffl_fast_load_init("DATA");
	floppy_iffl_fast_load(); 										// chars
	floppy_iffl_fast_load();										// palette
	floppy_iffl_fast_load();										// song

	fontsys_init();

	sdc_setbufferaddressmsb(0x04);									// set SDC buffer address to 0x0400
	sdc_setprocessdirentryfunc((uint16_t)(&main_processdirentry));	// set dir entry process pointer
	sdc_opendir();													// open and read root directory

	SEI
	setup_main();
	CLI

	while(1)
	{
		__asm
		(
			" lda 0xd020\n"
			" sta 0xd020\n"
		);
	}
}
