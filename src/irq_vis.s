			.rtmodel cpu, "*"
			
			.extern modplay_play
			.extern keyboard_update
			.extern fontsys_clearscreen
			.extern program_update_vis
			.extern _Zp

			.extern channel_tempvolume;
			.extern channel_sample;
			.extern channel_tempperiod;

zp_cadr		.equlab	_Zp + 40

samp1max	.byte 0
samp2max	.byte 0
samp3max	.byte 0
samp4max	.byte 0

samp1		.byte 0
samp2		.byte 0
samp3		.byte 0
samp4		.byte 0
sampcomb	.byte 0

samp1rnd	.byte 0
samp2rnd	.byte 0
samp3rnd	.byte 0
samp4rnd	.byte 0

; ------------------------------------------------------------------------------------

			.public irq_vis
irq_vis:
			php
			pha
			phx
			phy
			phz

			jsr modplay_play
			jsr program_update_vis
			jsr keyboard_update

			lda #0xff
			sta 0xd012
			lda #.byte0 irq_vis2
			sta 0xfffe
			lda #.byte1 irq_vis2
			sta 0xffff

			plz
			ply
			plx
			pla
			plp
			asl 0xd019
			rti

; ------------------------------------------------------------------------------------

irq_vis2:
			php
			pha
			phx
			phy
			phz

			lda 0xd012
waitr1$:	cmp 0xd012
			beq waitr1$

			lda #0x03
			sta 0xd021

			;lda #0xff
			;sta 0xd10f

			; test drawing colours into grid

			jsr map_colourram

			inc flipflop
			lda flipflop
			cmp #02
			bmi dontfade$
			lda #0
			sta flipflop
			jsr fadescreen
dontfade$:

			lda channel_tempperiod+1 ; lda #3
			lsr a
			lda channel_tempperiod+0 ; lda #3
			ror a
			clc
			adc #0x00
			tax
			lda vissine,x
			asl a
			sta squarex
			lda visrand1,x
			asl a
			sta squarew
			lda channel_sample+0
			jsr convertchannel
			clc
			adc samp1rnd
			sta squarec
			lda samp1rnd
			sta squareclo
			lda visrand2,x
			sta squareh
			lda viscosine,x
			sta squarey
			jsr drawsquare

			lda channel_tempperiod+3 ; lda #3
			lsr a
			lda channel_tempperiod+2 ; lda #3
			ror a
			clc
			adc #0x40
			tax
			lda vissine,x
			asl a
			sta squarex
			lda visrand1,x
			asl a
			sta squarew
			lda channel_sample+1
			jsr convertchannel
			clc
			adc samp2rnd
			sta squarec
			lda samp2rnd
			sta squareclo
			lda visrand2,x
			sta squareh
			lda viscosine,x
			sta squarey
			jsr drawsquare

			lda channel_tempperiod+5 ; lda #3
			lsr a
			lda channel_tempperiod+4 ; lda #3
			ror a
			clc
			adc #0x80
			tax
			lda vissine,x
			asl a
			sta squarex
			lda visrand1,x
			asl a
			sta squarew
			lda channel_sample+2
			jsr convertchannel
			clc
			adc samp3rnd
			sta squarec
			lda samp3rnd
			sta squareclo
			lda visrand2,x
			sta squareh
			lda viscosine,x
			sta squarey
			jsr drawsquare

			lda channel_tempperiod+7 ; lda #3
			lsr a
			lda channel_tempperiod+6 ; lda #3
			ror a
			clc
			adc #0xc0
			tax
			lda vissine,x
			asl a
			sta squarex
			lda visrand1,x
			asl a
			sta squarew
			lda channel_sample+3
			jsr convertchannel
			clc
			adc samp4rnd
			sta squarec
			lda samp4rnd
			sta squareclo
			lda visrand2,x
			sta squareh
			lda viscosine,x
			sta squarey
			jsr drawsquare


			; test gotox



			lda #0b00010000		; gotox
			sta 0x8370+0
			lda #0x00
			sta 0x8370+1

			inc amok+1

amok:
			lda #0x80			; gotox 128
			sta 0xa370+0
			lda #0x00
			sta 0xa370+1

			lda #0x20			; some character
			sta 0xa370+2
			lda #0x03
			sta 0xa370+3

			;lda #0x88
			;sta 0x8370+2
amok2:
			lda #0x82			; some colour for the character
			sta 0x8370+3


			lda #0b00010000		; gotox
			sta 0x8370+4
			lda #0x00
			sta 0x8370+5

			lda #0x40			; gotox 320
			sta 0xa370+4
			lda #0x01
			sta 0xa370+5


			jsr unmap_all

			; end test drawing colours into grid

			;lda #0x00
			;sta 0xd10f

			jsr analyzechannels

			lda #0x80
			sta 0xd012
			.public irqvec2
irqvec2:
			lda #.byte0 irq_vis
			sta 0xfffe
			.public irqvec3
irqvec3:
			lda #.byte1 irq_vis
			sta 0xffff

			plz
			ply
			plx
			pla
			plp
			asl 0xd019
			rti

; ------------------------------------------------------------------------------------

fadescreen:
			lda #.byte0 0x8001
			sta zp_cadr+0
			lda #.byte1 0x8001
			sta zp_cadr+1

			ldx #25
			ldy #0x00
fs1$:		lda (zp_cadr),y
			and #0xf0
			sta tempme
			lda (zp_cadr),y
			and #0x0f
			cmp #1
			beq fs2$
			sec
			sbc #1
fs2$:		ora tempme
			sta (zp_cadr),y
			iny
			iny
			cpy #80
			bne fs1$

			clc
			lda zp_cadr+0
			adc #160
			sta zp_cadr+0
			lda zp_cadr+1
			adc #0
			sta zp_cadr+1
			
			dex
			bne fs1$

			rts

tempme		.byte 0
flipflop	.byte 0

; ------------------------------------------------------------------------------------

drawsquare:

			ldy squarey
			lda times80tablelo,y
			clc
			adc #0x01
			sta ds2$+1
			lda times80tablehi,y
			clc
			adc #0x80
			sta ds2$+2

			clc
			lda ds2$+1
			adc squarex
			sta ds2$+1
			sta ds3$+1
			lda ds2$+2
			adc #0
			sta ds2$+2
			sta ds3$+2

			ldy #0
ds1$:		ldx #0
ds2$:		lda 0x8001,x
			and #0x0f
			cmp squareclo
			bpl ds4$
			lda squarec
ds3$:		sta 0x8001,x
ds4$:		inx
			inx
			cpx squarew
			bne ds2$

			clc
			lda ds2$+1
			adc #160
			sta ds2$+1
			sta ds3$+1
			lda ds2$+2
			adc #0
			sta ds2$+2
			sta ds3$+2

			iny
			cpy squareh
			bne ds1$

			; draw glowing outline

			dec squarex ; decx2 because each char is 2 bytes
			dec squarex
			dec squarey

			lda squareclo
			lsr a
			lsr a
			sta squareclo

			lda squarec
			and #0xf0
			ora squareclo
			sta squarec

			ldy squarey
			lda times80tablelo,y
			clc
			adc #0x01
			sta ds6$+1
			lda times80tablehi,y
			clc
			adc #0x80
			sta ds6$+2

			clc
			lda squarew
			adc #0x04
			sta squarew
			clc
			lda squareh
			adc #0x02
			sta squareh

			clc
			lda ds6$+1
			adc squarex
			sta ds6$+1
			sta ds7$+1
			lda ds6$+2
			adc #0
			sta ds6$+2
			sta ds7$+2

			ldy #0
ds5$:		ldx #0
ds6$:		lda 0x8001,x
			and #0x0f
			cmp squareclo
			bpl ds8$
			lda squarec
ds7$:		sta 0x8001,x
ds8$:		inx
			inx
			cpx squarew
			bne ds6$

			clc
			lda ds6$+1
			adc #160
			sta ds6$+1
			sta ds7$+1
			lda ds6$+2
			adc #0
			sta ds6$+2
			sta ds7$+2

			iny
			cpy squareh
			bne ds5$

			rts

squarex:	.byte 0
squarey:	.byte 0
squarew:	.byte 0
squareh:	.byte 0

squarec:	.byte 0
squareclo:	.byte 0

; ------------------------------------------------------------------------------------

analyzechannels:

			ldx #0
			ldz #0x00
			lda #0x00
			sta zp_cadr+3
			sta 0xd770
			sta 0xd771
			sta 0xd772
			sta 0xd773
			sta 0xd774
			sta 0xd775
			sta 0xd776
			sta 0xd777

			; decrease sampmax somewhat		
			sec
			lda samp1max
			sbc #0x20
			bcs b1$
			lda #0x00
b1$:		sta samp1max

			sec
			lda samp2max
			sbc #0x20
			bcs b2$
			lda #0x00
b2$:		sta samp2max

			sec
			lda samp3max
			sbc #0x20
			bcs b3$
			lda #0x00
b3$:		sta samp3max

			sec
			lda samp4max
			sbc #0x20
			bcs b4$
			lda #0x00
b4$:		sta samp4max

loopsamplerend$:

			; get current sample strengths, convert to unsigned and store in samp
			lda 0xd72a
			sta zp_cadr+0
			lda 0xd72b
			sta zp_cadr+1
			lda 0xd72c
			sta zp_cadr+2
			lda [zp:zp_cadr],z
			bpl ge1$
			eor 0xff
ge1$:		sta samp1

			lda 0xd73a
			sta zp_cadr+0
			lda 0xd73b
			sta zp_cadr+1
			lda 0xd73c
			sta zp_cadr+2
			lda [zp:zp_cadr],z
			bpl ge2$
			eor 0xff
ge2$:		sta samp2

			lda 0xd74a
			sta zp_cadr+0
			lda 0xd74b
			sta zp_cadr+1
			lda 0xd74c
			sta zp_cadr+2
			lda [zp:zp_cadr],z
			bpl ge3$
			eor 0xff
ge3$:		sta samp3

			lda 0xd75a
			sta zp_cadr+0
			lda 0xd75b
			sta zp_cadr+1
			lda 0xd75c
			sta zp_cadr+2
			lda [zp:zp_cadr],z
			bpl ge4$
			eor 0xff
ge4$:		sta samp4

			; multiply samples with current channel volumes
			lda channel_tempvolume+0
			sta 0xd770
			lda samp1
			sta 0xd774
			lda 0xd779
			sta samp1

			lda channel_tempvolume+1
			sta 0xd770
			lda samp2
			sta 0xd774
			lda 0xd779
			sta samp2

			lda channel_tempvolume+2
			sta 0xd770
			lda samp3
			sta 0xd774
			lda 0xd779
			sta samp3

			lda channel_tempvolume+3
			sta 0xd770
			lda samp4
			sta 0xd774
			lda 0xd779
			sta samp4

			; bigger than sampmax?
			lda samp1max
			cmp samp1
			bcs nl1$
			lda samp1
			sta samp1max
nl1$:

			lda samp2max
			cmp samp2
			bcs nl2$
			lda samp2
			sta samp2max
nl2$:

			lda samp3max
			cmp samp3
			bcs nl3$
			lda samp3
			sta samp3max
nl3$:

			lda samp4max
			cmp samp4
			bcs nl4$
			lda samp4
			sta samp4max
nl4$:


			lda 0xd012
waitr4$:	cmp 0xd012
			beq waitr4$

			inx
			cpx #32
			beq loopsamplerendend$
			jmp loopsamplerend$

loopsamplerendend$:

			clc
			lda samp1max
			lsr a
			adc #1
			cmp #0x10
			bmi lsre1$
			lda #0x0f
lsre1$:		sta samp1rnd

			lda samp2max
			lsr a
			adc #1
			cmp #0x10
			bmi lsre2$
			lda #0x0f
lsre2$:		sta samp2rnd

			lda samp3max
			lsr a
			adc #1
			cmp #0x10
			bmi lsre3$
			lda #0x0f
lsre3$:		sta samp3rnd

			lda samp4max
			lsr a
			adc #1
			cmp #0x10
			bmi lsre4$
			lda #0x0f
lsre4$:		sta samp4rnd

			rts

; ------------------------------------------------------------------------------------

waitlines:
			lda 0xd012
wl$			cmp 0xd012
			beq wl$
			dey
			bne waitlines

			lda #0x00
			sta 0xd10f
			sta 0xd20f
			sta 0xd30f

			rts

; ------------------------------------------------------------------------------------

convertchannel:
			sec
			sbc #1
			and #0x03
			asl a
			asl a
			asl a
			asl a
			clc
			adc #0x80
			rts

rendercolour:
			lda 0xc000,y
			sta 0xd10f

			lda 0xc100,y
			sta 0xd20f

			lda 0xc200,y
			sta 0xd30f

			rts

; ------------------------------------------------------------------------------------

/*
swapnybbles:

			asl a						; swap nybbles
			adc #0x80
			rol a
			asl a
			adc #0x80
			rol a
			rts
*/

; ------------------------------------------------------------------------------------

map_colourram
			lda #0xff
			ldx #0b00000000
			ldy #0xff
			ldz #0b00001111
			map

			lda #0x00
			ldx #0b00000000
			ldy #0x88
			ldz #0x17
			map
			eom

			rts

unmap_all
			lda #0x00
			tax
			tay
			taz
			map
			nop

			rts

; ------------------------------------------------------------------------------------

			.public times80tablelo
times80tablelo:

			.byte ( 0*160) & 0xff
			.byte ( 1*160) & 0xff
			.byte ( 2*160) & 0xff
			.byte ( 3*160) & 0xff
			.byte ( 4*160) & 0xff
			.byte ( 5*160) & 0xff
			.byte ( 6*160) & 0xff
			.byte ( 7*160) & 0xff
			.byte ( 8*160) & 0xff
			.byte ( 9*160) & 0xff
			.byte (10*160) & 0xff
			.byte (11*160) & 0xff
			.byte (12*160) & 0xff
			.byte (13*160) & 0xff
			.byte (14*160) & 0xff
			.byte (15*160) & 0xff
			.byte (16*160) & 0xff
			.byte (17*160) & 0xff
			.byte (18*160) & 0xff
			.byte (19*160) & 0xff
			.byte (20*160) & 0xff
			.byte (21*160) & 0xff
			.byte (22*160) & 0xff
			.byte (23*160) & 0xff
			.byte (24*160) & 0xff

			.public times80tablehi
times80tablehi:

			.byte (( 0*160) >> 8) & 0xff
			.byte (( 1*160) >> 8) & 0xff
			.byte (( 2*160) >> 8) & 0xff
			.byte (( 3*160) >> 8) & 0xff
			.byte (( 4*160) >> 8) & 0xff
			.byte (( 5*160) >> 8) & 0xff
			.byte (( 6*160) >> 8) & 0xff
			.byte (( 7*160) >> 8) & 0xff
			.byte (( 8*160) >> 8) & 0xff
			.byte (( 9*160) >> 8) & 0xff
			.byte ((10*160) >> 8) & 0xff
			.byte ((11*160) >> 8) & 0xff
			.byte ((12*160) >> 8) & 0xff
			.byte ((13*160) >> 8) & 0xff
			.byte ((14*160) >> 8) & 0xff
			.byte ((15*160) >> 8) & 0xff
			.byte ((16*160) >> 8) & 0xff
			.byte ((17*160) >> 8) & 0xff
			.byte ((18*160) >> 8) & 0xff
			.byte ((19*160) >> 8) & 0xff
			.byte ((20*160) >> 8) & 0xff
			.byte ((21*160) >> 8) & 0xff
			.byte ((22*160) >> 8) & 0xff
			.byte ((23*160) >> 8) & 0xff
			.byte ((24*160) >> 8) & 0xff

vissine:
    .byte   17,  20,  18,  17,  16,  17,  19,  20,  18,  19,  20,  21,  21,  20,  21,  19
    .byte   22,  23,  19,  20,  23,  20,  21,  21,  23,  23,  24,  23,  25,  23,  22,  21
    .byte   22,  22,  23,  24,  22,  23,  23,  22,  22,  25,  24,  26,  26,  24,  26,  25
    .byte   26,  23,  27,  23,  26,  27,  23,  26,  26,  23,  27,  25,  26,  26,  24,  25
    .byte   25,  27,  24,  24,  25,  23,  24,  23,  25,  25,  27,  24,  23,  24,  26,  27
    .byte   27,  26,  24,  23,  24,  25,  24,  25,  23,  23,  22,  26,  25,  22,  25,  23
    .byte   23,  25,  23,  23,  24,  22,  23,  22,  23,  23,  20,  21,  21,  19,  20,  19
    .byte   19,  19,  19,  19,  22,  18,  17,  20,  19,  19,  21,  18,  18,  20,  16,  17
    .byte   16,  15,  15,  15,  18,  17,  17,  14,  14,  14,  17,  14,  15,  14,  15,  16
    .byte   16,  15,  16,  14,  13,  12,  15,  15,  12,  11,  14,  12,  12,  13,  12,  14
    .byte   11,  10,  12,  13,   9,   9,  10,   9,  13,   9,  13,  11,  11,   8,  12,   8
    .byte    8,  10,   9,   8,  12,   9,   8,  11,  12,  11,   9,  12,   8,  11,  10,   8
    .byte   11,   8,  10,  12,  11,   9,   9,   8,   9,   8,  12,  10,  11,   8,  12,   8
    .byte   11,  11,   9,   8,  12,  10,   9,   9,   9,  12,  12,  12,  12,  13,  13,  11
    .byte   13,  14,  14,  11,  13,  11,  11,  13,  12,  15,  13,  13,  12,  14,  12,  12
    .byte   12,  16,  14,  14,  16,  14,  17,  17,  14,  15,  16,  16,  16,  16,  16,  19

viscosine:
    .byte   19,  20,  20,  18,  18,  20,  18,  16,  19,  20,  17,  20,  19,  18,  17,  16
    .byte   17,  19,  20,  17,  19,  15,  18,  18,  17,  15,  15,  16,  17,  18,  14,  16
    .byte   16,  18,  15,  16,  18,  16,  13,  14,  17,  16,  13,  12,  16,  16,  15,  14
    .byte   15,  13,  12,  15,  14,  15,  10,  12,  10,  11,  13,  11,  13,  10,  10,   9
    .byte   10,  10,  11,  12,  12,   8,  10,  10,   7,   9,  11,  10,   6,   8,   9,   7
    .byte    7,   5,   5,   9,   6,   5,   7,   5,   6,   6,   4,   8,   7,   7,   5,   3
    .byte    3,   7,   7,   4,   2,   2,   4,   2,   4,   4,   3,   3,   1,   1,   1,   1
    .byte    5,   5,   2,   5,   3,   2,   1,   4,   2,   5,   4,   3,   2,   1,   2,   3
    .byte    5,   2,   4,   3,   5,   5,   3,   4,   1,   2,   1,   3,   2,   3,   2,   5
    .byte    3,   3,   3,   4,   1,   5,   3,   2,   6,   5,   6,   5,   5,   3,   5,   6
    .byte    6,   5,   5,   4,   3,   4,   4,   8,   5,   7,   5,   6,   9,   8,   6,   8
    .byte    6,   8,   9,   7,   8,  10,   7,   9,  10,   7,  10,   8,   8,   8,  10,  11
    .byte   10,   9,  12,  12,  11,  12,  10,  10,  13,  13,  13,  12,  12,  12,  11,  14
    .byte   14,  16,  14,  13,  16,  15,  14,  14,  13,  16,  15,  17,  18,  17,  17,  18
    .byte   18,  17,  14,  19,  19,  16,  18,  18,  19,  18,  19,  19,  16,  19,  16,  16
    .byte   20,  19,  20,  16,  20,  16,  18,  16,  19,  16,  19,  16,  17,  20,  18,  16

visrand1:
    .byte    6,   2,   7,   3,   5,   4,   5,   7,   5,   3,   4,   5,   7,   5,   3,   4
    .byte    6,   4,   6,   5,   2,   6,   2,   7,   5,   3,   5,   7,   2,   5,   7,   5
    .byte    4,   6,   3,   5,   3,   4,   5,   2,   2,   3,   3,   2,   2,   7,   2,   4
    .byte    7,   3,   7,   3,   4,   3,   5,   6,   2,   4,   3,   6,   6,   5,   4,   2
    .byte    7,   2,   4,   7,   7,   5,   5,   6,   6,   6,   7,   6,   4,   3,   7,   3
    .byte    2,   5,   4,   2,   4,   5,   4,   3,   3,   3,   3,   2,   2,   6,   4,   2
    .byte    7,   2,   3,   2,   7,   2,   4,   3,   7,   5,   4,   4,   5,   3,   4,   4
    .byte    7,   4,   6,   6,   6,   6,   2,   2,   5,   5,   3,   5,   3,   6,   3,   5
    .byte    4,   4,   6,   7,   7,   5,   3,   5,   3,   2,   2,   2,   3,   3,   7,   3
    .byte    2,   4,   3,   5,   7,   5,   3,   7,   3,   3,   7,   2,   7,   3,   7,   2
    .byte    4,   4,   4,   2,   5,   5,   2,   4,   5,   2,   3,   7,   5,   7,   6,   3
    .byte    2,   2,   2,   5,   7,   2,   3,   4,   3,   7,   7,   7,   3,   3,   4,   5
    .byte    4,   6,   5,   5,   4,   4,   7,   6,   7,   3,   5,   2,   3,   3,   2,   4
    .byte    4,   6,   3,   2,   2,   6,   4,   5,   6,   4,   5,   7,   7,   6,   4,   7
    .byte    6,   6,   4,   4,   3,   4,   4,   7,   6,   5,   6,   5,   2,   4,   6,   3
    .byte    6,   3,   3,   3,   6,   4,   7,   6,   5,   2,   5,   2,   6,   4,   5,   6

visrand2:
    .byte    5,   2,   4,   6,   2,   4,   4,   2,   5,   2,   5,   3,   4,   3,   6,   2
    .byte    4,   5,   4,   4,   5,   4,   5,   6,   7,   2,   4,   7,   5,   2,   2,   6
    .byte    6,   6,   5,   5,   4,   6,   4,   7,   7,   5,   4,   5,   6,   3,   4,   3
    .byte    5,   6,   5,   4,   6,   4,   5,   7,   2,   7,   3,   6,   7,   6,   2,   4
    .byte    7,   2,   5,   2,   4,   3,   5,   3,   6,   4,   7,   2,   3,   2,   5,   3
    .byte    3,   6,   6,   3,   7,   3,   7,   6,   6,   3,   4,   5,   4,   5,   4,   7
    .byte    6,   4,   4,   7,   6,   4,   3,   6,   5,   7,   6,   6,   4,   7,   7,   4
    .byte    3,   6,   5,   2,   5,   7,   7,   5,   3,   3,   4,   4,   7,   6,   5,   6
    .byte    3,   7,   6,   2,   6,   4,   4,   2,   2,   6,   4,   3,   5,   4,   7,   7
    .byte    7,   6,   7,   7,   2,   4,   5,   4,   3,   5,   4,   3,   2,   5,   3,   7
    .byte    3,   6,   7,   6,   5,   2,   4,   5,   3,   5,   3,   6,   7,   6,   5,   6
    .byte    6,   7,   7,   2,   5,   6,   5,   5,   4,   4,   6,   4,   4,   5,   3,   5
    .byte    7,   5,   3,   3,   4,   4,   5,   3,   2,   5,   7,   5,   3,   6,   4,   4
    .byte    4,   5,   5,   7,   6,   7,   5,   6,   6,   2,   2,   2,   3,   4,   7,   6
    .byte    5,   5,   7,   5,   5,   4,   5,   4,   3,   5,   2,   5,   5,   5,   2,   3
    .byte    4,   2,   2,   5,   2,   5,   5,   7,   3,   5,   5,   2,   7,   4,   6,   7