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

/*
	STORED FAT dir entry:

	$00		asciiz		The long file name
	$40		byte		The length of long file name
	$41		ascii		The ”8.3” file name.
						The name part is padded with spaces to make it exactly 8 bytes. The 3 bytes of the extension follow. There is no . between the name and the extension. There is no NULL byte.
	$4e		dword		The cluster number where the file begins. For sub-directories, this is where the FAT dir entries start for that sub-directory.
	$52		dword		The length of file in bytes.
	$56		byte		The type and attribute bits.

	$57		asciiz		converted file name (0x40 bytes)
	$97		10			10 bytes for filesize string
*/

#define DIR_ENTRY_SIZE			0x57
#define STORED_DIR_ENTRY_SIZE	0xa1

uint32_t	direntryoffset	= 0;
uint32_t	rowoffset = 0;
uint16_t	numdirentries	= 0;
uint8_t		program_keydowncount = 0;
uint8_t		program_keydowndelay = 0;
int16_t		program_dir_selectedrow = 0;
uint8_t*	program_transbuf;

uint8_t		xemu_fudge = 8;

void program_processdirentry()
{
	if(program_transbuf[0x00] == 0x2e)
	{
		if(program_transbuf[0x01] == 0x00) // "." - dont process
			return;

		if(program_transbuf[0x01] == 0x2e)
			if(program_transbuf[0x02] == 0x00) // ".." - dont process
				return;
	}

	// convert ascii name to font chars and append to end of entry structure
	for(uint16_t i = 0; i < 64; i++)
		program_transbuf[0x57+i] = fontsys_asciitofont[program_transbuf[i]];

	// get filesize and convert to filesize string
	poke(&fnts_bin + 0, program_transbuf[0x52 + 0]);
	poke(&fnts_bin + 1, program_transbuf[0x52 + 1]);
	poke(&fnts_bin + 2, program_transbuf[0x52 + 2]);
	poke(&fnts_bin + 3, program_transbuf[0x52 + 3]);
	fontsys_convertfilesizetostring();
	for(uint16_t i = 0; i < 10; i++)
		program_transbuf[0x57+64+i] = ((uint8_t *)&fnts_binstring)[i];

	dma_dmacopy(program_transbuf, ATTICDIRENTRIES + direntryoffset, STORED_DIR_ENTRY_SIZE);

	poke(0xc000 + numdirentries, numdirentries);

	direntryoffset += STORED_DIR_ENTRY_SIZE;

	numdirentries++;
}


void program_sortdirentries()
{
	int n = numdirentries;
	uint8_t swapped;

	// sort directories to front
	for(uint32_t i = 0; i < n - 1; i++)
	{
		swapped = 0;
		for(uint16_t j = 0; j < n - i - 1; j++)
		{
			uint32_t addr1 = (ATTICDIRENTRIES + STORED_DIR_ENTRY_SIZE * peek(0xc000+j+0));
			uint32_t addr2 = (ATTICDIRENTRIES + STORED_DIR_ENTRY_SIZE * peek(0xc000+j+1));

			uint8_t val1 = (lpeek(addr1 + 0x56) & 0b00010000);
			uint8_t val2 = (lpeek(addr2 + 0x56) & 0b00010000);

			if (val1 < val2)
			{
				uint8_t foo = peek(0xc000 + j);
				poke(0xc000+j, peek(0xc000+j+1));
				poke(0xc000+j+1, foo);
				swapped = 1;
			}
		}

		// If no two elements were swapped, then break
		if (swapped == 0)
			break;
	}

	// sort names
	for(uint32_t i = 0; i < n - 1; i++)
	{
		swapped = 0;
		for(uint16_t j = 0; j < n - i - 1; j++)
		{
			uint32_t addr1 = (ATTICDIRENTRIES + STORED_DIR_ENTRY_SIZE * peek(0xc000+j+0));
			uint32_t addr2 = (ATTICDIRENTRIES + STORED_DIR_ENTRY_SIZE * peek(0xc000+j+1));

			uint8_t val1 = (lpeek(addr1 + 0x56) & 0b00010000);
			uint8_t val2 = (lpeek(addr2 + 0x56) & 0b00010000);
			uint8_t val3 = (lpeek(addr1));
			uint8_t val4 = (lpeek(addr2));

			if (val1 == val2 && val3 > val4)
			{
				uint8_t foo = peek(0xc000 + j);
				poke(0xc000+j, peek(0xc000+j+1));
				poke(0xc000+j+1, foo);
				swapped = 1;
			}
		}

		// If no two elements were swapped, then break
		if (swapped == 0)
			break;
	}

/*
	for(uint32_t i = 0; i < numdirentries; i++)
	{
		uint32_t addr1 = (ATTICDIRENTRIES + STORED_DIR_ENTRY_SIZE * peek(0xc000+i));
		uint8_t val1 = lpeek(addr1);
		poke(0xc000+i, val1);
	}
*/
}


void program_loaddata()
{
	fl_init();
	fl_waiting();
	
	floppy_iffl_fast_load_init("DATA");
	floppy_iffl_fast_load(); 										// chars
	floppy_iffl_fast_load();										// palette

	// chars are loaded to 0x08100000 in attic ram. copy it back to normal ram, location 0x10000
	dma_dmacopy(ATTICFONTCHARMEM, FONTCHARMEM, 0x8000);
}

void program_settransbuf()
{
	sdc_setbufferaddressmsb(0x04);									// set SDC buffer address to 0x0400
	program_transbuf = (uint8_t *)(sdc_transferbuffermsb * 256);
}

void program_opendir()
{
	direntryoffset = 0;
	numdirentries = 0;
	program_dir_selectedrow = 0;
	sdc_setprocessdirentryfunc((uint16_t)(&program_processdirentry));	// set dir entry process pointer
	sdc_opendir();													// open and read root directory

	program_sortdirentries();
}

void program_chdir()
{
	sdc_chdir();
}

void program_openfile()
{
	// load mod into attic (max $8000000 - $8050000)
	poke(&sdc_loadaddresslo,  (uint8_t)((0x000000 >>  0) & 0xff));
	poke(&sdc_loadaddressmid, (uint8_t)((0x000000 >>  8) & 0xff));
	poke(&sdc_loadaddresshi,  (uint8_t)((0x000000 >> 16) & 0xff));

	sdc_hyppo_loadfile_attic();
}

void program_init()
{
	// TODO - bank in correct palette
	// TODO - create DMA job for this
	for(uint8_t i = 0; i < 255; i++)
	{
		poke(0xd100+i, ((uint8_t *)PALETTE)[0*256+i]);
		poke(0xd200+i, ((uint8_t *)PALETTE)[1*256+i]);
		poke(0xd300+i, ((uint8_t *)PALETTE)[2*256+i]);
	}
	// TODO - set correct palette

	VIC2.BORDERCOL = 0x0f;
	VIC2.SCREENCOL = 0x00;

	modplay_disable();

	fontsys_init();
	fontsys_clearscreen();

	program_settransbuf();
	program_opendir();												// open and read root directory
}

void program_drawdirectory()
{
	fontsys_map();

	uint16_t numrows = numdirentries;

	rowoffset = 0;

	int16_t startrow = 12-program_dir_selectedrow;
	if(startrow < 0)
	{
		rowoffset = -startrow;
		startrow = 0;
	}

	int16_t endrow = startrow + numrows;
	if(endrow > 25)
		endrow = 25;

	if(numdirentries - program_dir_selectedrow < 13)
	{
		endrow = 12 + (numdirentries - program_dir_selectedrow);
	}

	direntryoffset = rowoffset; // * STORED_DIR_ENTRY_SIZE;
	for(uint16_t row = startrow; row < endrow; row++)
	{
		uint32_t direntry = peek(0xc000+direntryoffset) * STORED_DIR_ENTRY_SIZE;
		dma_dmacopy(ATTICDIRENTRIES + DIR_ENTRY_SIZE - 1 - 4 + direntry, (uint32_t)&fnts_tempbuf, 0x40 + 1 + 4 + 10);	// -4 for file length, -1 for attribute, +10 for filesize string

		fnts_row = 2 * row;
		fnts_column = 0;
		
		uint8_t attrib = peek(&fnts_tempbuf + 4);
		uint8_t color = 0x0f;
		if((attrib & 0b00010000) == 0b00010000)
			color = 0x2f;

		poke(&fnts_curpal + 1, color);

		fontsys_asm_setupscreenpos();
		fontsys_asm_render();
		fontsys_asm_rendergotox();
		fontsys_asm_renderfilesize();

		direntryoffset ++; // = STORED_DIR_ENTRY_SIZE;
	}

	fontsys_unmap();
}

void program_processkeyboard()
{
	if(xemu_fudge > 0)
	{
		xemu_fudge--;
		return;
	}

	if(keyboard_keypressed(KEYBOARD_CURSORDOWN) == 1)
	{
		program_keydowndelay--;
		if(program_keydowndelay == 0)
			program_keydowndelay = 1;

		if(program_keydowncount == 0)
			program_dir_selectedrow++;

		if(program_dir_selectedrow >= numdirentries)
			program_dir_selectedrow = numdirentries - 1;

		program_keydowncount++;
		if(program_keydowncount > program_keydowndelay)
			program_keydowncount = 0;
	}
	else if(keyboard_keypressed(KEYBOARD_CURSORUP) == 1)
	{
		program_keydowndelay--;
		if(program_keydowndelay == 0)
			program_keydowndelay = 1;

		if(program_keydowncount == 0)
			program_dir_selectedrow--;

		if(program_dir_selectedrow < 0)
			program_dir_selectedrow = 0;

		program_keydowncount++;
		if(program_keydowncount > program_keydowndelay)
			program_keydowncount = 0;
	}
	else if(keyboard_keyreleased(KEYBOARD_RETURN))
	{
		uint32_t direntry = peek(0xc000+program_dir_selectedrow);

		// get current entry (without converted filename bit)
		dma_dmacopy(ATTICDIRENTRIES + direntry * STORED_DIR_ENTRY_SIZE, program_transbuf, DIR_ENTRY_SIZE);	

		uint8_t attrib = program_transbuf[0x56];
	
		if((attrib & 0b00010000) == 0b00010000)
		{
			program_chdir();
			program_opendir();
		}
		else
		{
			modplay_disable();
			program_openfile();
			modplay_init(ATTICADDRESS, SAMPLEADRESS);
			modplay_enable();
		}
	}
	else if(keyboard_keyreleased(KEYBOARD_ESC))
	{
		modplay_disable();
	}
	else if(keyboard_keyreleased(KEYBOARD_INSERTDEL))
	{
		poke(program_transbuf+0x00, 0x2e);
		poke(program_transbuf+0x01, 0x2e);
		poke(program_transbuf+0x02, 0x00);

		poke(program_transbuf+0x41, 0x2e);
		poke(program_transbuf+0x42, 0x2e);
		poke(program_transbuf+0x43, 0x00);

		program_chdir();
		program_opendir();
	}
	else
	{
		program_keydowndelay = 32;
		program_keydowncount = 0;
	}
}

void program_update()
{
	program_drawdirectory();
	program_processkeyboard();
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