#include <stdint.h>
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
#include "dmajobs.h"
#include "program.h"

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

uint8_t*	direntryptr		= (uint8_t *)0x7000;
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

void program_loaddata()
{
	fl_init();
	fl_waiting();
	
	floppy_iffl_fast_load_init("DATA");
	floppy_iffl_fast_load(); 										// chars
	floppy_iffl_fast_load();										// palette
	floppy_iffl_fast_load();										// song
}

void program_init()
{
	run_dma_job((__far char *)&dma_clearcolorram1);
	run_dma_job((__far char *)&dma_clearcolorram2);
	run_dma_job((__far char *)&dma_clearscreen1);
	run_dma_job((__far char *)&dma_clearscreen2);

   	modplay_init(0x20000);

	fontsys_init();

	sdc_setbufferaddressmsb(0x04);									// set SDC buffer address to 0x0400
	sdc_setprocessdirentryfunc((uint16_t)(&main_processdirentry));	// set dir entry process pointer
	sdc_opendir();													// open and read root directory

	// TODO - bank in correct palette
	// TODO - create DMA job for this
	for(uint8_t i=0; i<255; i++)
	{
		poke(0xd100+i, ((uint8_t *)PALETTE)[0*256+i]);
		poke(0xd200+i, ((uint8_t *)PALETTE)[1*256+i]);
		poke(0xd300+i, ((uint8_t *)PALETTE)[2*256+i]);
	}
	// TODO - set correct palette

	VIC2.BORDERCOL = 0x0f;
	VIC2.SCREENCOL = 0x00;
}

void program_mainloop()
{
	while(1)
	{
		__asm
		(
			" lda 0xd020\n"
			" sta 0xd020\n"
		);
	}
}