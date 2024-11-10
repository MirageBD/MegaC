; ----------------------------------------------------------------------------------------------------

							.public sdc_loadaddresslo
sdc_loadaddresslo			.byte 0x00
							.public sdc_loadaddressmid
sdc_loadaddressmid			.byte 0x00
							.public sdc_loadaddresshi
sdc_loadaddresshi			.byte 0x00

							.public sdc_transferbuffermsb
sdc_transferbuffermsb		.byte 0x04						; MSB of transfer area

							.public sdc_filedescriptor
sdc_filedescriptor			.byte 0x00

sdc_currentsector			.long 0x00000000

sdc_chunk_filesize			.long 0x00000000
sdc_chunk_filesizeexcess	.long 0x00000000

; ----------------------------------------------------------------------------------------------------

SDC_READSECTOR_ASYNC	.macro
							lda 0x02
							sta 0xd680
						.endm

SDC_MAPSECTORBUFFER		.macro
							lda 0x01			; Clear colour RAM at $DC00 flag, as this prevents mapping of sector buffer at $DE00
							trb 0xd030

							lda 0x81
							sta 0xd680
						.endm

SDC_UNMAPSECTORBUFFER	.macro
							lda 0x82
							sta 0xd680
						.endm

MACROWITHARGS			.macro a, b
							lda \a
							sta \b
						.endm

; ----------------------------------------------------------------------------------------------------

/* error codes:
$01 1 partition not interesting				The partition is not of a supported type.
$02 2 bad signature							The signature bytes at the end of a partition table or of the first sector of a partition were missing or incorrect.
$03 3 is small FAT							This is partition is FAT12 or FAT16 partition. Only FAT32 is supported.
$04 4 too many reserved clusters			The partition has more than 65,535 reserved sectors.
$05 5 not two FATs							The partition does not have exactly two copies of the FAT structure.
$06 6 too few clusters						The partition contains too few clusters.
$07 7 read timeout							It took to long to read from the SD card.
$08 8 partition error						An unspecified error occurred while handling a partition.
$10 16 invalid address						An invalid address was supplied in an argument.
$11 17 illegal value						An illegal value was supplied in an argument.
$20 32 read error							An unspecified error occurred while reading.
$21 33 write error							An unspecified error occurred while writing.
$80 128 no such drive						The supplied Hyppo drive number does not exist.
$81 129 name too long						The supplied filename was too long.
$82 130 not implemented						The Hyppo service is not implemented.
$83 131 file too long						The file is larger than 16MB.
$84 132 too manyopen files					All of the file descriptors are in use.
$85 133 invalid cluster						The supplied cluster number is invalid.
$86 134 is a directory						An attempt was made to operate on a directory, where a normal file was expected.
$87 135 not a directory						An attempt was made to operate on a normal file, where a directory was expected.
$88 136 file not found						The file could not be located in the current directory of the current drive.
$89 137 invalid file descriptor				An invalid or closed file descriptor was supplied.
$8A 138 image wrong length					The disk image file has the wrong length.
$8B 139 image fragmented					The disk image is not stored contiguously on the SD card.
$8C 140 no space							The SD card has no free space for the requested operation.
$8D 141 file exists							A file already exists with the given name.
$8E 142 directory full						The directory cannot accommodate any more entries.
$FF 255 eof									The end of a file or directory was encountered.
$FF 255 no such trap						There is no Hyppo service available for the trap. The program may be incompatible with this version of Hyppo.
*/

		.public sdc_asm_geterror
sdc_asm_geterror:

		lda 0x38
		sta 0xd640
		clv
		sta 0xc001
		rts

; ----------------------------------------------------------------------------------------------------

		.public sdc_asm_opendir
sdc_asm_opendir:

		lda #0x00
		sta 0xd640
		nop
		ldz #0x00

		lda #0x12										; hyppo_opendir - open the current working directory
		sta 0xd640
		clv
		bcc sdc_opendir_error

		tax												; transfer the directory file descriptor into X
		ldy sdc_transferbuffermsb						; set Y to the MSB of the transfer area

sdcod1:
		lda #0x14										; hyppo_readdir - read the directory entry
		sta 0xd640
		clv
		bcc sdcod2

		phx
		phy
		.public sdc_processdirentryptr
sdc_processdirentryptr:
		jsr 0xbabe										; call function that handles the retrieved filename
		ply
		plx
		bra sdcod1

sdcod2:	cmp #0x85										; if the error code in A is 0x85 we have reached the end of the directory otherwise there’s been an error
		bne sdc_opendir_error
		lda #0x16										; close the directory file descriptor in X
		sta 0xd640
		clv
		bcc sdc_opendir_error
		rts

sdc_opendir_error:
		lda #0x38
		sta 0xd640
		clv

sdc_opendir_errorloop:
		lda #0x06
		sta 0xd020
		lda #0x07
		sta 0xd020
		jmp sdc_opendir_errorloop

; ----------------------------------------------------------------------------------------------------

		.public sdc_asm_chdir
sdc_asm_chdir

		ldy sdc_transferbuffermsb						; set the hyppo filename from transferbuffer
		lda #0x2e
		sta 0xd640
		clv
		bcc sdc_chdirend
		lda #0x34										; find the FAT dir entry
		sta 0xd640
		clv
		bcc sdc_chdirend
		lda #0x0c										; chdir into the directory
		sta 0xd640
		clv
		rts

sdc_chdirend:
		rts

; ----------------------------------------------------------------------------------------------------

		; caution: there's sdc_asm_openfile and sdc_asm_HYPPO_loadfile

		.public sdc_asm_openfile
sdc_asm_openfile:

		ldy sdc_transferbuffermsb						; set the hyppo filename from transferbuffer
		lda 0x2e										; hyppo_setname
		sta 0xd640
		clv
		bcc sdc_asm_hypposetname_error

		lda 0x34										; hyppo_findfile
		sta 0xd640
		clv
		bcc sdc_asm_hyppofindfile_error

		lda 0x18										; hyppo_openfile
		sta 0xd640
		clv
		bcc sdc_asm_openfile_error

		sta sdc_filedescriptor

		lda 0x00
		sta sdc_chunk_filesize+0
		sta sdc_chunk_filesize+1
		sta sdc_chunk_filesize+2
		sta sdc_chunk_filesize+3
		sta sdc_chunk_filesizeexcess+0
		sta sdc_chunk_filesizeexcess+1
		sta sdc_chunk_filesizeexcess+2
		sta sdc_chunk_filesizeexcess+3

		rts

sdc_asm_hypposetname_error:
		lda 0x01
		sta 0xc000
		jsr sdc_asm_geterror
errorloop1$:
		lda 0x10
		sta 0xd020
		lda 0x20
		sta 0xd020
		jmp errorloop1$

sdc_asm_hyppofindfile_error
		lda 0x02
		sta 0xc000
		jsr sdc_asm_geterror
errorloop2$:
		lda 0x10
		sta 0xd020
		lda 0x20
		sta 0xd020
		jmp errorloop2$

sdc_asm_openfile_error:
		lda 0x03
		sta 0xc000
		jsr sdc_asm_geterror
errorloop3$:
		lda 0x10
		sta 0xd020
		lda 0x20
		sta 0xd020
		jmp errorloop3$

; ----------------------------------------------------------------------------------------------------

sdc_asm_hyppo_loadfile_common:

		ldy sdc_transferbuffermsb						; set the hyppo filename from transferbuffer
		lda #0x2e
		sta 0xd640
		clv
		bcc sdc_asm_hyppo_loadfileend

		ldx sdc_loadaddresslo
		ldy sdc_loadaddressmid
		ldz sdc_loadaddresshi

		rts

; ----------------------------------------------------------------------------------------------------

		; caution: there's sdc_asm_openfile and sdc_asm_HYPPO_loadfile

		.public sdc_asm_hyppo_loadfile
sdc_asm_hyppo_loadfile:

		jsr sdc_asm_hyppo_loadfile_common
		lda #0x36										; 0x36 hyppo_loadfile for chip RAM at 0x00ZZYYXX
		sta 0xd640										; Mega65.HTRAP00
		clv												; Wasted instruction slot required following hyper trap instruction
		bcc sdc_asm_hyppo_loadfileend
		rts

sdc_asm_hyppo_loadfileend:
		rts

; ----------------------------------------------------------------------------------------------------

		.public sdc_asm_hyppo_loadfile_attic
sdc_asm_hyppo_loadfile_attic:

		jsr sdc_asm_hyppo_loadfile_common
		lda #0x3e										; 0x36 hyppo_loadfile for chip RAM at 0x00ZZYYXX
		sta 0xd640										; Mega65.HTRAP00
		clv												; Wasted instruction slot required following hyper trap instruction
		bcc sdc_asm_hyppo_loadfileend
		rts

; ----------------------------------------------------------------------------------------------------

		.public sdc_asm_hyppoclosefile
sdc_asm_hyppoclosefile:

		ldx sdc_filedescriptor
		lda 0x20										; Preconditions: The file descriptor given in the X register was opened using hyppo_openfile.
		sta 0xd640
		clv
		;bcc sdc_asm_hyppoclosefile_error

		rts

sdc_asm_hyppoclosefile_error:
		lda 0x04
		sta 0xc000
		jsr sdc_asm_geterror
errorloop$:
		lda 0x10
		sta 0xd020
		lda 0x20
		sta 0xd020
		jmp errorloop$

; ----------------------------------------------------------------------------------------------------

		.public sdc_asm_readfirstsector
sdc_asm_readfirstsector:

		; SDC_ASM_READFIRSTSECTOR is only used once using a hypervisor trap.
		; this is so the correct sector/track/etc. are being set.

														; assume the file is already open.		
		lda 0xd030										; unmap the colour RAM from $dc00 because that will prevent us from mapping in the sector buffer
		pha
		and 0b11111110
		sta 0xd030										; first bit of d030 is CRAM2K

		lda 0x1a										; read the next sector (hyppo_readfile)
		sta 0xd640
		clv

		cpx 0x00										; WHY DO I HAVE TO DO THIS??? ERROR HANDLING SHOULD TAKE CARE OF THIS!!!
		bne cont1$
		cpy 0x00
		bne cont1$
		clc
		jmp sdc_asm_readfirstsector_done

cont1$:	bcs cont2$
		jmp sdc_asm_readfirstsector_error

cont2$:

sdc_asm_readfirstsector_done:

		pla												; map the colour RAM at $dc00 if it was previously mapped
		sta 0xd030

		lda 0xd681
		sta sdc_currentsector+0
		lda 0xd682
		sta sdc_currentsector+1
		lda 0xd683
		sta sdc_currentsector+2
		lda 0xd684
		sta sdc_currentsector+3

		rts

sdc_asm_readfirstsector_error:

		cmp 0xff										; if the error code in A is $ff we have reached the end of the file otherwise there’s been an error
		bne sdc_asm_readfirstsector_fatalerror

		pla												; map the colour RAM at $dc00 if it was previously mapped
		sta 0xd030

		lda 0x34
		sta 0x01

		rts

sdc_asm_readfirstsector_fatalerror:

errorloop$:
		inc 0xd020
		jmp errorloop$

; ----------------------------------------------------------------------------------------------------

		.public sdc_asm_chunk_readasync
sdc_asm_chunk_readasync

		lda 0xd680										; test if sdc is ready
		and 0x03
		beq sdc_asm_chunk_sdcready
		rts

sdc_asm_chunk_sdcready:									; sdc is ready.

sdc_asm_chunk_readnext:

		lda 0x02										; start async read of sector
		sta 0xd680

		rts

; ----------------------------------------------------------------------------------------------------
