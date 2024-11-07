#ifndef _HYPPO_H
#define _HYPPO_H

#define HYPPOTRAP(a, x, y, z)	__asm(" lda " ## a);
//    	__asm(" lda #"a"\n" " ldx #"x"\n" " ldy #"y"\n" " ldz #"z"\n" " sta $d640\n" " clv");

/*
hyppo_getversion			// 0x00 // same as hyppo_rom_writeprotect?
hyppo_getdefaultdrive		// 0x02
hyppo_getcurrentdrive		// 0x04
hyppo_selectdrive			// 0x06
hyppo_getdrivesize			// 0x08
hyppo_getcwd				// 0x0a
hyppo_chdir					// 0x0c
hyppo_mkdir					// 0x0e
hyppo_rmdir					// 0x10
hyppo_opendir				// 0x12
hyppo_readdir				// 0x14
hyppo_closedir				// 0x16
hyppo_openfile				// 0x18
hyppo_readfile				// 0x1a
hyppo_writefile				// 0x1c
hyppo_mkfile				// 0x1e
hyppo_closefile				// 0x20
hyppo_closeall				// 0x22
hyppo_seekfile				// 0x24
hyppo_rmfile				// 0x26
hyppo_fstat					// 0x28
hyppo_rename				// 0x2a
hyppo_filedate				// 0x2c
hyppo_setname				// 0x2e
hyppo_findfirst				// 0x30
hyppo_findnext				// 0x32
hyppo_findfile				// 0x34
hyppo_loadfile				// 0x36
hyppo_geterrorcode			// 0x38
hyppo_setup_transfer_area	// 0x3a
hyppo_cdrootdir				// 0x3c
hyppo_loadfile_attic		// 0x3e
hyppo_d81attach0			// 0x40
hyppo_d81detach				// 0x42
hyppo_d81write_en			// 0x44
hyppo_d81attach1			// 0x46

hyppo_get_mapping			// 0x74
hyppo_set_mapping			// 0x76

hyppo_reset					// 0x7e
hyppo_rom_writeenable		// 0x02       // USE THIS IN MAIN?!?
hyppo_rom_writeprotect		// 0x00
*/

#endif