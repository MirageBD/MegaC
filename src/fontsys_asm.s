; ----------------------------------------------------------------------------------------------------

fnts_numchars	.equ (1600/16)		; 100 chars, so should only have to bother setting lower value in screenram

; ----------------------------------------------------------------------------------------------------

		.public fontsys_init
fontsys_init:

		ldx #0x00
fs_i0$:
		lda #0b00001000	; NCM bit
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
		lda #0b00000100				; overflowed, so we have a trimhi
		ora fnts_chartrimshi,x
		sta fnts_chartrimshi,x
fs_il$:	inx
		cpx #fnts_numchars
		bne fs_i0$

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

; ----------------------------------------------------------------------------------------------------
