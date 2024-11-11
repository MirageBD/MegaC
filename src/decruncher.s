			.rtmodel version,"1"
			.rtmodel codeModel,"plain"
			.rtmodel core,"45gs02"
			.rtmodel target,"mega65"
			.extern _Zp

; -----------------------------------------------------------------------------------------------

dc_bits:	.equlab _Zp
dc_get_zp:	.equlab	_Zp + 1

; -----------------------------------------------------------------------------------------------

addput:		clc
			tya
			adc dc_ldst+0
			sta dc_ldst+0
			bcc next1$
			lda dc_ldst+1
			adc #0x00
			sta dc_ldst+1
			bcc next1$
			lda dc_ldst+2
			adc #0x00
			sta dc_ldst+2

next1$:		clc
			tya
			adc dc_mdst+0
			sta dc_mdst+0
			bcc next2$
			lda dc_mdst+1
			adc #0x00
			sta dc_mdst+1
			bcc next2$
			lda dc_mdst+2
			adc #0x00
			sta dc_mdst+2
next2$:		rts

addget:		clc
			tya
			adc zp:dc_get_zp+0
			sta zp:dc_get_zp+0
			bcc next$
			lda zp:dc_get_zp+1
			adc #0x00
			sta zp:dc_get_zp+1
			bcc next$
			lda zp:dc_get_zp+2
			adc #0x00
			sta zp:dc_get_zp+2
next$:		rts

getlen:		lda #1
glloop:		jsr getnextbit
			bcc glend
			jsr rolnextbit									; if next bit is 1 then rol a the next-next bit into A
			bpl glloop										; if the highest bit is now still 0, continue. this means highest len is 255
glend:		rts

rolnextbit:	jsr getnextbit
			rol a											; rol sets N flag
			rts

getnextbit:
			asl zp:dc_bits
			bne dgend
			pha
			jsr getnextbyte
			rol a
			sta zp:dc_bits
			pla
dgend:		rts

getnextbyte:
			lda [zp:dc_get_zp],z
			inc zp:dc_get_zp+0
			bne next$
			inc zp:dc_get_zp+1
			bne next$
			inc zp:dc_get_zp+2
next$:		rts

; -----------------------------------------------------------------------------------------------

offsets:	.byte 0b11011111 ; 3							; short offsets
			.byte 0b11111011 ; 6
			.byte 0b00000000 ; 8
			.byte 0b10000000 ; 10
			.byte 0b11101111 ; 4							; long offsets
			.byte 0b11111101 ; 7
			.byte 0b10000000 ; 10
			.byte 0b11110000 ; 13

; -----------------------------------------------------------------------------------------------

			.section code,text
			.public decrunch
decrunch:	ldz #0x00
			jsr getnextbyte									; set unpack address
			sta dc_ldst+0
			sta dc_mdst+0
			jsr getnextbyte
			sta dc_ldst+1
			sta dc_mdst+1
			jsr getnextbyte
			sta dc_ldst+2
			sta dc_mdst+2
			jsr getnextbyte									; set attic byte (megabyte). normally a >>20 shift, so shift left 4 bytes to get to 3*8
			asl a
			asl a
			asl a
			asl a
			sta dc_lsrcm+1
			sta dc_msrcm+1
			sta dc_ldstm+1
			sta dc_mdstm+1

			clc

			lda #0x80
			sta zp:dc_bits

dloop:		jsr getnextbit									; after this, carry is 0, bits = 01010101
			bcs match

			jsr getlen										; Literal run.. get length. after this, carry = 0, bits = 10101010, A = 1
			sta dc_llen
			tay												; put length into y for addput

			lda zp:dc_get_zp+0
			sta dc_lsrc+0
			lda zp:dc_get_zp+1
			sta dc_lsrc+1
			lda zp:dc_get_zp+2
			sta dc_lsrc+2

			sta 0xd707										; inline DMA copy
dc_lsrcm:	.byte 0x80, 0x00								; sourcebank
dc_ldstm:	.byte 0x81, 0x00								; destbank
			.byte 0x00										; end of job options
			.byte 0x00										; copy
dc_llen:	.word 0x0000									; count
dc_lsrc:	.word 0x0000									; src
			.byte 0x00										; src bank
dc_ldst:	.word 0x0000									; dst
			.byte 0x00										; dst bank
			.byte 0x00										; cmd hi
			.word 0x0000									; modulo, ignored

			jsr addget
			jsr addput

			iny	
			beq dloop
															; has to continue with a match so fall through
match:		jsr getlen										; match.. get length.

			tax												; length 255 -> EOF
			inx
			beq dc_end

			stx dc_mlen

			lda #0											; Get num bits
			cpx #3
			rol a
			jsr rolnextbit
			jsr rolnextbit
			tax
			lda offsets,x
			beq m8

loop$:		jsr rolnextbit									; Get bits < 8
			bcs loop$
			bmi mshort

m8:			eor #0xff										; Get byte
			tay
			jsr getnextbyte
			jmp mdone

			;.byte 0xae ; = jmp mdone (ldx 0xffa0)

mshort:		ldy #0xff

mdone:		;clc

			ldx #0x00										; HRMPF! HAVE TO DO THIS NASTY SHIT TO WORK AROUND DMA BUG :(((((
			cmp #0xff										; compare A with ff
			bne next1$
			cpy #0xff										; compare Y with ff
			bne next1$
			ldx #0b00000010									; FFFF = -1 offset -> set source addressing to HOLD
next1$:		stx dc_cmdh

			clc
			adc dc_mdst+0
			sta dc_msrc+0
			tya
			adc dc_mdst+1
			sta dc_msrc+1

			lda dc_mdst+2									; added for m65 for when we cross banks
			sta dc_msrc+2
			bcs next2$
			dec dc_msrc+2

next2$:		sta 0xd707										; inline DMA copy
dc_msrcm:	.byte 0x80, 0x00								; sourcebank
dc_mdstm:	.byte 0x81, 0x00								; destbank
			.byte 0x00										; end of job options
			.byte 0x00										; copy
dc_mlen:	.word 0x0000									; count
dc_msrc:	.word 0x0000									; src
			.byte 0x00										; src bank and flags
dc_mdst:	.word 0x0000									; dst
			.byte 0x00										; dst bank and flags
dc_cmdh:	.byte 0x00										; cmd hi
			.word 0x0000									; modulo, ignored

			ldy dc_mlen
			jsr addput

			jmp dloop

dc_end:		rts

; -----------------------------------------------------------------------------------------------

