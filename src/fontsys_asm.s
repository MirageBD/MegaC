; ----------------------------------------------------------------------------------------------------

			.extern _Zp

; ----------------------------------------------------------------------------------------------------

fnts_numchars	.equ (1600/16)		; 100 chars, so should only have to bother setting lower value in screenram
fontcharmem		.equ 0x10000

zpscrdst1:		.equlab _Zp + 80
zpscrdst2:		.equlab _Zp + 82
zpcoldst1:		.equlab _Zp + 84
zpcoldst2:		.equlab _Zp + 86

; ----------------------------------------------------------------------------------------------------

				.public fnts_row
fnts_row		.byte 0
				.public fnts_column
fnts_column		.byte 0

				.public fnts_tempbuf
fnts_tempbuf	.space 0x4f			; $40 for filename + $0a for filesize string + $04 filesize + $01 attribute

; ----------------------------------------------------------------------------------------------------

		.public fontsys_asm_init
fontsys_asm_init:

		ldx #0x00
fs_i0$:
		lda #0b00001000				; NCM bit, no 8-pixel trim
		sta fnts_chartrimshi,x
		sec
		lda #16
		sbc fnts_charwidths,x
		asl a						; upper 3 bits are trimlo
		asl a
		asl a
		asl a
		asl a
		sta fnts_chartrimslo,x
		bcc fs_il$
		lda #0b00001100				; overflowed, so we have a trimhi. ora with NCM bit and store
		sta fnts_chartrimshi,x
fs_il$:	inx
		cpx #128
		bne fs_i0$

		rts

; ----------------------------------------------------------------------------------------------------

		.public fontsys_asm_setupscreenpos
fontsys_asm_setupscreenpos:

fnts_readrow:
		ldy fnts_row

		lda fnts_screentablo+0,y
		sta zp:zpscrdst1+0
		lda fnts_screentabhi+0,y
		sta zp:zpscrdst1+1

		lda fnts_screentablo+1,y
		sta zp:zpscrdst2+0
		lda fnts_screentabhi+1,y
		sta zp:zpscrdst2+1

		lda fnts_attribtablo+0,y
		sta zp:zpcoldst1+0
		lda fnts_attribtabhi+0,y
		sta zp:zpcoldst1+1

		lda fnts_attribtablo+1,y
		sta zp:zpcoldst2+0
		lda fnts_attribtabhi+1,y
		sta zp:zpcoldst2+1

		rts

; ----------------------------------------------------------------------------------------------------

		.public fontsys_asm_render
fontsys_asm_render:

		ldy fnts_column

		ldx #0

		.public fnts_readchar
fnts_readchar:
		lda fnts_tempbuf+5,x
		beq fontsys_asmrender_end

		phx
		tax

		sta (zp:zpscrdst1),y
		clc
		adc #.byte0 (fontcharmem / 64 + 1 * fnts_numchars) ; 64
		sta (zp:zpscrdst2),y

		lda fnts_chartrimshi,x
		sta (zp:zpcoldst1),y
		sta (zp:zpcoldst2),y

		iny

		lda #.byte1 (fontcharmem / 64)
		ora fnts_chartrimslo,x
		sta (zp:zpscrdst1),y
		sta (zp:zpscrdst2),y

		.public fnts_curpal
fnts_curpal:
		lda #0x0f ; palette 0 and colour 15 for transparent pixels
		sta (zp:zpcoldst1),y
		sta (zp:zpcoldst2),y

		iny

		plx
		inx
		bra fnts_readchar

fontsys_asmrender_end:

		rts

; ----------------------------------------------------------------------------------------------------

		.public fontsys_asm_renderfilesize
fontsys_asm_renderfilesize:

		ldx #0x00
		ldy #2*61

fontsys_asm_renderfilesizeloop:
		lda fnts_tempbuf+5+64,x
		sta (zp:zpscrdst1),y
		clc
		adc #.byte0 (fontcharmem / 64 + 1 * fnts_numchars) ; 64
		sta (zp:zpscrdst2),y

		iny
		iny

		inx
		cpx #10
		bne fontsys_asm_renderfilesizeloop

		rts

; ----------------------------------------------------------------------------------------------------

		.public fontsys_asm_rendergotox
fontsys_asm_rendergotox:

		ldy #2*60

		lda #0b00010000				; set gotox
		sta (zp:zpcoldst1),y
		sta (zp:zpcoldst2),y

		lda #.byte0 400
		sta (zp:zpscrdst1),y
		sta (zp:zpscrdst2),y

		iny

		lda #0						; clear pixel row mask flags
		sta (zp:zpcoldst1),y
		sta (zp:zpcoldst2),y

		lda #.byte1 400
		sta (zp:zpscrdst1),y
		sta (zp:zpscrdst2),y

		iny

		rts

; ----------------------------------------------------------------------------------------------------

					.public fnts_charwidths
fnts_charwidths:	;      .   !   "   #   $   %   &   '   (   )   *   +   ,   -   .   /   0   1   2   3
					.byte  4,  3,  7,  9,  9, 10,  9,  3,  5,  5,  8,  6,  4,  6,  4,  7,  9,  6,  7,  8
					;      4   5   6   7   8   9   :   ;   <   =   >   ?   @   a   b   c   d   e   f   g
					.byte  9,  8,  8,  9,  8,  9,  4,  4,  6,  7,  6,  8,  9,  8,  7,  8,  8,  8,  5,  8
					;      h   i   j   k   l   m   n   o   p   q   r   s   t   u   v   w   x   y   z   [
					.byte  7,  3,  4,  7,  3, 11,  7,  8,  8,  8,  5,  7,  4,  7,  7, 12,  8,  8,  7,  4
					;      £   ]   |   -   -   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O
					.byte  9,  4,  8,  8,  8, 10,  8, 10,  9,  7,  6, 10,  8,  3,  7,  8,  6, 13,  8, 10
					;      P   Q   R   S   T   U   V   W   X   Y   Z   =   =   =   -   o   .   .   ~   .
					.byte  8, 11,  8,  9,  7,  9, 10, 14,  9,  9,  8, 10, 16,  6, 16,  8,  4,  4,  8,  4

					.byte  4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4

					.public fnts_chartrimshi
fnts_chartrimshi:	.space 128
					.public fnts_chartrimslo
fnts_chartrimslo:	.space 128

					.public fnts_screentablo
fnts_screentablo:	.space 50		; .byte <(screen          + rrbscreenwidth2 * I)
					.public fnts_screentabhi
fnts_screentabhi:	.space 50		; .byte >(screen          + rrbscreenwidth2 * I)
					.public fnts_attribtablo
fnts_attribtablo:	.space 50		; .byte <(mappedcolourmem + rrbscreenwidth2 * I)
					.public fnts_attribtabhi
fnts_attribtabhi:	.space 50		; .byte >(mappedcolourmem + rrbscreenwidth2 * I)

; ----------------------------------------------------------------------------------------------------

			.public fnts_bin
fnts_bin	.long  0x00ffffff	; A test value to convert (LSB first)
			.public fnts_bcd
fnts_bcd	.space 5			; Should end up as $45,$23,$01

			.public fnts_binstring
fnts_binstring
			.space 10

			.public fontsys_convertfilesizetostring
fontsys_convertfilesizetostring:

			sed					; Switch to decimal mode
			lda #0				; Ensure the result is clear
			sta fnts_bcd+0
			sta fnts_bcd+1
			sta fnts_bcd+2
			sta fnts_bcd+3
			sta fnts_bcd+4

			ldx #32				; The number of source bits
       
cnvbit:		asl fnts_bin+0		; Shift out one bit
			rol fnts_bin+1
			rol fnts_bin+2
			rol fnts_bin+3

			lda fnts_bcd+0		; And add into result
			adc fnts_bcd+0
			sta fnts_bcd+0
			lda fnts_bcd+1		; propagating any carry
			adc fnts_bcd+1
			sta fnts_bcd+1
			lda fnts_bcd+2		; ... thru whole result
			adc fnts_bcd+2
			sta fnts_bcd+2
			lda fnts_bcd+3		; ... thru whole result
			adc fnts_bcd+3
			sta fnts_bcd+3
			lda fnts_bcd+4		; ... thru whole result
			adc fnts_bcd+4
			sta fnts_bcd+4

			dex					; And repeat for next bit
			bne cnvbit
			
			cld					; Back to binary

			ldy #0
			ldx #4

tostr:		lda fnts_bcd,x
			lsr a
			lsr a
			lsr a
			lsr a
			sta fnts_binstring,y

			lda fnts_bcd,x
			and #0x0f
			sta fnts_binstring+1,y
			iny
			iny
			dex
			bpl tostr

			ldx #0x00
trimstart:	lda fnts_binstring,x
			beq trimstart2
			bra trimstart3
trimstart2:	lda #0x63
			sta fnts_binstring,x
			inx
			cpx #10
			bne trimstart
			bra trimstartend

trimstart3:	lda fnts_binstring,x
			clc
			adc #0x10
			sta fnts_binstring,x
			inx
			cpx #10
			bne trimstart3

trimstartend:
			rts

; ----------------------------------------------------------------------------------------------------
