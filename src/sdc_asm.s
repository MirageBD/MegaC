; ----------------------------------------------------------------------------------------------------

						.public sdc_loadaddresslo
sdc_loadaddresslo		.byte 0x00
						.public sdc_loadaddressmid
sdc_loadaddressmid		.byte 0x00
						.public sdc_loadaddresshi
sdc_loadaddresshi		.byte 0x00

						.public sdc_transferbuffermsb
sdc_transferbuffermsb	.byte 0x04						; MSB of transfer area

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

sdc_openfile:

		ldy sdc_transferbuffermsb						; set the hyppo filename from transferbuffer
		lda #0x2e
		sta 0xd640
		clv
		bcc sdc_openfileend

		ldx sdc_loadaddresslo
		ldy sdc_loadaddressmid
		ldz sdc_loadaddresshi

		lda #0x36										; 0x36 for chip RAM at 0x00ZZYYXX
		sta 0xd640										; Mega65.HTRAP00
		clv												; Wasted instruction slot required following hyper trap instruction
		bcc sdc_openfileend

		rts

sdc_openfileend:
		rts

; ----------------------------------------------------------------------------------------------------