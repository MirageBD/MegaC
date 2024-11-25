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

extern void irq_main();
extern void irq_vis();

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

void start_main_irq()
{
	VIC3.H640			= 1;
	VIC3.V400			= 1;
	VIC4.DISPROWS		= 50;
	VIC4.CHRYSCL		= 0;

	VIC4.CHRCOUNTLSB	= RRBSCREENWIDTH;
	VIC4.CHRCOUNTMSB	= RRBSCREENWIDTH >> 8;
	VIC4.LINESTEP		= RRBSCREENWIDTH2;

	VIC4.TEXTXPOSLSB	= 0x90;

	VIC4.ALPHEN			= 0;

	// THIS IS NOT THE PROBLEM OF THE RANDOM VERTICAL OFFSET
	fontsys_map();
	dma_runjob((__far char *)&dma_clearcolorram1);
	dma_runjob((__far char *)&dma_clearcolorram2);
	fontsys_unmap();
	dma_runjob((__far char *)&dma_clearscreen1);
	dma_runjob((__far char *)&dma_clearscreen2);

	poke(&irqvec0+1, (uint8_t)(((uint16_t)&irq_main >> 0) & 0xff));
	poke(&irqvec1+1, (uint8_t)(((uint16_t)&irq_main >> 8) & 0xff));
	poke(&irqvec2+1, (uint8_t)(((uint16_t)&irq_main >> 0) & 0xff));
	poke(&irqvec3+1, (uint8_t)(((uint16_t)&irq_main >> 8) & 0xff));
}

void start_vis_irq()
{
	VIC3.H640			= 0;
	VIC3.V400			= 0;
	VIC4.DISPROWS		= 25;
	VIC4.CHRYSCL		= 1;

	//VIC4.CHRCOUNTLSB	= 40;
	//VIC4.CHRCOUNTMSB	= 0; // 40 >> 8;
	//VIC4.LINESTEP		= 40 * 2;

	VIC4.CHRCOUNTLSB	= RRBSCREENWIDTH;
	VIC4.CHRCOUNTMSB	= RRBSCREENWIDTH >> 8;
	VIC4.LINESTEP		= RRBSCREENWIDTH2;

	VIC4.TEXTXPOSLSB	= 80;

	VIC4.ALPHEN			= 1;

	// THIS IS NOT THE PROBLEM OF THE RANDOM VERTICAL OFFSET
	fontsys_map();
	dma_runjob((__far char *)&dma_visualizer_clearcolorram1);
	dma_runjob((__far char *)&dma_visualizer_clearcolorram2);
	fontsys_unmap();
	dma_runjob((__far char *)&dma_visualizer_clearscreen1);
	dma_runjob((__far char *)&dma_visualizer_clearscreen2);

	poke(&irqvec0+1, (uint8_t)(((uint16_t)&irq_vis >> 0) & 0xff));
	poke(&irqvec1+1, (uint8_t)(((uint16_t)&irq_vis >> 8) & 0xff));
	poke(&irqvec2+1, (uint8_t)(((uint16_t)&irq_vis >> 0) & 0xff));
	poke(&irqvec3+1, (uint8_t)(((uint16_t)&irq_vis >> 8) & 0xff));
}

#define DIR_ENTRY_SIZE			0x57
#define STORED_DIR_ENTRY_SIZE	0xa1

uint32_t	direntryoffset	= 0;
uint32_t	rowoffset = 0;
uint16_t	numdirentries	= 0;
uint8_t		program_keydowncount = 0;
uint8_t		program_keydowndelay = 0;
int16_t		program_dir_selectedrow = 0;
uint8_t*	program_transbuf;

uint16_t	program_jukebox_entry = 0;
uint8_t		program_jukebox_playing = 0;

uint8_t		xemu_fudge = 8;

uint8_t program_entryisdir()
{
		uint8_t attrib = program_transbuf[0x56];
		if((attrib & 0b00010000) == 0b00010000)
			return 1;
		return 0;
}

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

	poke(DIRENTPTRS + numdirentries, numdirentries);

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
			uint32_t addr1 = (ATTICDIRENTRIES + STORED_DIR_ENTRY_SIZE * peek(DIRENTPTRS+j+0));
			uint32_t addr2 = (ATTICDIRENTRIES + STORED_DIR_ENTRY_SIZE * peek(DIRENTPTRS+j+1));

			uint8_t val1 = (lpeek(addr1 + 0x56) & 0b00010000);
			uint8_t val2 = (lpeek(addr2 + 0x56) & 0b00010000);

			if (val1 < val2)
			{
				uint8_t foo = peek(DIRENTPTRS + j);
				poke(DIRENTPTRS + j, peek(DIRENTPTRS + j + 1));
				poke(DIRENTPTRS + j + 1, foo);
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
			uint32_t addr1 = (ATTICDIRENTRIES + STORED_DIR_ENTRY_SIZE * peek(DIRENTPTRS + j + 0));
			uint32_t addr2 = (ATTICDIRENTRIES + STORED_DIR_ENTRY_SIZE * peek(DIRENTPTRS + j + 1));

			uint8_t val1 = (lpeek(addr1 + 0x56) & 0b00010000);
			uint8_t val2 = (lpeek(addr2 + 0x56) & 0b00010000);

			if(val1 == val2)
			{
				for(uint32_t k = 0; k < 64; k++)
				{
					uint8_t val3 = (lpeek(addr1 + k));
					uint8_t val4 = (lpeek(addr2 + k));

					if(val3 < val4) // already smaller?
					{
						k = 64;		// in right order, quit
					}
					else if(val3 > val4) // bigger, so swap
					{
						uint8_t foo = peek(DIRENTPTRS + j);
						poke(DIRENTPTRS + j, peek(DIRENTPTRS + j + 1));
						poke(DIRENTPTRS + j + 1, foo);
						swapped = 1;
						k = 64;
					}

					// val3 == val4 -> continue testing
				}
			}
		}

		// If no two elements were swapped, then break
		if (swapped == 0)
			break;
	}
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

uint8_t testchar[64] = 
{
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0,
	0xff, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0,
	0xff, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0,
	0xff, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0,
	0xff, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0,
	0xff, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0,
	0xff, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0,
};

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
	VIC2.SCREENCOL = 0x0f;

	for(uint8_t i = 0; i < 64; i++)
	{
		poke(0xc800+i, testchar[i]);
	}

	modplay_init();
	fontsys_init();
	fontsys_clearscreen();

	program_settransbuf();
	program_opendir();												// open and read root directory

	VIC2.SCREENCOL = 0x00;
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

	direntryoffset = rowoffset;
	for(uint16_t row = startrow; row < endrow; row++)
	{
		uint32_t direntry = peek(DIRENTPTRS + direntryoffset) * STORED_DIR_ENTRY_SIZE;
		dma_dmacopy(ATTICDIRENTRIES + DIR_ENTRY_SIZE - 1 - 4 + direntry, (uint32_t)&fnts_tempbuf, 0x40 + 1 + 4 + 10);	// -4 for file length, -1 for attribute, +10 for filesize string

		fnts_row = 2 * row;
		fnts_column = 0;
		
		uint8_t attrib = peek(&fnts_tempbuf + 4);
		uint8_t color = 0x0f;
		if((attrib & 0b00010000) == 0b00010000)
			color = 0x1f;

		poke(&fnts_curpal + 1, color);

		fontsys_asm_setupscreenpos();
		fontsys_asm_render();
		fontsys_asm_rendergotox();
		fontsys_asm_renderfilesize();

		direntryoffset++;
	}

	fontsys_unmap();
}

void program_main_processkeyboard()
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
		uint32_t direntry = peek(DIRENTPTRS + program_dir_selectedrow);

		// get current entry (without converted filename bit)
		dma_dmacopy(ATTICDIRENTRIES + direntry * STORED_DIR_ENTRY_SIZE, program_transbuf, DIR_ENTRY_SIZE);	

		if(program_entryisdir())
		{
			program_chdir();
			program_opendir();
		}
		else
		{
			mp_loop = 1;
			modplay_mute();
			modplay_disable();
			program_openfile();
			modplay_initmod(ATTICADDRESS, SAMPLEADRESS);
			start_vis_irq();
			modplay_enable();
		}
	}
	else if(keyboard_keyreleased(KEYBOARD_F3))
	{
			mp_done = 1;
			mp_loop = 0;
			modplay_mute();
			modplay_disable();
			program_jukebox_playing = 1;
			program_jukebox_entry = program_dir_selectedrow; // start playing from currently selected
			start_vis_irq();
	}
	else if(keyboard_keyreleased(KEYBOARD_ESC))
	{
		mp_done = 1;
		modplay_mute();
		modplay_disable();
		program_jukebox_playing = 0;
		program_jukebox_entry = 0;
		start_main_irq();
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

void program_vis_processkeyboard()
{
	if(keyboard_keyreleased(KEYBOARD_ESC))
	{
		mp_done = 1;
		modplay_mute();
		modplay_disable();
		program_jukebox_playing = 0;
		program_jukebox_entry = 0;
		start_main_irq();
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
	program_main_processkeyboard();
}

void program_update_vis()
{
	program_vis_processkeyboard();

	if(program_jukebox_playing)
	{
		if(mp_done)
		{
			modplay_mute();
			modplay_disable();

			uint32_t direntry = peek(DIRENTPTRS + (program_jukebox_entry % numdirentries));
			dma_dmacopy(ATTICDIRENTRIES + direntry * STORED_DIR_ENTRY_SIZE, program_transbuf, DIR_ENTRY_SIZE);	

			while(program_entryisdir())
			{
				program_jukebox_entry++;
				direntry = peek(DIRENTPTRS + (program_jukebox_entry % numdirentries));
				dma_dmacopy(ATTICDIRENTRIES + direntry * STORED_DIR_ENTRY_SIZE, program_transbuf, DIR_ENTRY_SIZE);	
			}

			program_dir_selectedrow = program_jukebox_entry;

			program_openfile();
			modplay_initmod(ATTICADDRESS, SAMPLEADRESS);

			modplay_enable();

			program_jukebox_entry = (program_jukebox_entry + 13) % numdirentries;
		}
	}
}

void program_mainloop()
{
	while(1)
	{
		__asm
		(
			" lda 0xd020\n"
			" sta 0xd020\n"
			" lda 0xd020\n"
			" sta 0xd020\n"
			" lda 0xd020\n"
			" sta 0xd020\n"
			" lda 0xd020\n"
			" sta 0xd020\n"
		);
	}
}