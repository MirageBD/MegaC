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
		cpx #fnts_numchars
		bne fs_i0$

		rts

; ----------------------------------------------------------------------------------------------------

		.public fontsys_asm_test
fontsys_asm_test:

		ldx #0

fnts_readrow:
		lda fnts_row
		tay

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

fnts_readcolumn:
		lda fnts_column
		tay

		.public fnts_readchar
fnts_readchar:
		lda 0xbabe,x
		beq fontsys_asm_test_end

		phx
		tax

		sta (zp:zpscrdst1),y
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

fontsys_asm_test_end:
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
					;      P   Q   R   S   T   U   V   W   X   Y   Z   =   =   =   -   o   .   .   .   .
					.byte  8, 11,  8,  9,  7,  9, 10, 14,  9,  9,  8, 10, 16,  6, 16,  8, 16, 16, 16, 16

					.public fnts_chartrimshi
fnts_chartrimshi:	.space 100
					.public fnts_chartrimslo
fnts_chartrimslo:	.space 100

					.public fnts_screentablo
fnts_screentablo:	.space 50		; .byte <(screen          + rrbscreenwidth2 * I)
					.public fnts_screentabhi
fnts_screentabhi:	.space 50		; .byte >(screen          + rrbscreenwidth2 * I)
					.public fnts_attribtablo
fnts_attribtablo:	.space 50		; .byte <(mappedcolourmem + rrbscreenwidth2 * I)
					.public fnts_attribtabhi
fnts_attribtabhi:	.space 50		; .byte >(mappedcolourmem + rrbscreenwidth2 * I)

; ----------------------------------------------------------------------------------------------------
