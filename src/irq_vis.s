			.rtmodel cpu, "*"
			
			.extern modplay_play
			.extern keyboard_update
			.extern fontsys_clearscreen
			.extern program_update_vis
			.extern _Zp

			.extern channel_tempvolume;
			.extern channel_sample;

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
			;jsr fontsys_clearscreen
			jsr program_update_vis
			jsr keyboard_update

			lda #0x32
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

			lda #0x80
			sta 0xd021

			;lda #0xff
			;sta 0xd10f

			; test drawing colours into grid

			jsr map_colourram

			lda #3
			sta squareh

			lda #3
			asl a
			sta squarex
			lda #4
			asl a
			sta squarew
			lda channel_sample+0
			jsr convertchannel
			clc
			adc samp1rnd
			sta squarec
			ldy #4
			jsr drawsquare

			lda #6
			asl a
			sta squarex
			lda #4
			asl a
			sta squarew
			lda channel_sample+1
			jsr convertchannel
			clc
			adc samp2rnd
			sta squarec
			ldy #9
			jsr drawsquare

			lda #9
			asl a
			sta squarex
			lda #4
			asl a
			sta squarew
			lda channel_sample+2
			jsr convertchannel
			clc
			adc samp3rnd
			sta squarec
			ldy #14
			jsr drawsquare

			lda #12
			asl a
			sta squarex
			lda #4
			asl a
			sta squarew
			lda channel_sample+3
			jsr convertchannel
			clc
			adc samp4rnd
			sta squarec
			ldy #19
			jsr drawsquare

			jsr unmap_all

			; end test drawing colours into grid

			;lda #0x00
			;sta 0xd10f

			jsr analyzechannels

			lda #0x08
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

drawsquare:

			lda times80tablelo,y
			clc
			adc #0x01
			sta ds$+1
			lda times80tablehi,y
			clc
			adc #0x80
			sta ds$+2

			clc
			lda ds$+1
			adc squarex
			sta ds$+1
			lda ds$+2
			adc #0
			sta ds$+2

			ldy #0
ds0$:		lda squarec
			ldx #0
ds$:		sta 0x8001,x
			inx
			inx
			cpx squarew
			bne ds$

			clc
			lda ds$+1
			adc #80
			sta ds$+1
			lda ds$+2
			adc #0
			sta ds$+2

			iny
			cpy squareh
			bne ds0$

			rts

squarex:	.byte 0
squarey:	.byte 0
squarew:	.byte 0
squareh:	.byte 0

squarec:	.byte 0

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
			lda samp1
			lsr a
			adc #1
			cmp #0x10
			bmi lsre1$
			lda #0x0f
lsre1$:		sta samp1rnd

			lda samp2
			lsr a
			adc #1
			cmp #0x10
			bmi lsre2$
			lda #0x0f
lsre2$:		sta samp2rnd

			lda samp3
			lsr a
			adc #1
			cmp #0x10
			bmi lsre3$
			lda #0x0f
lsre3$:		sta samp3rnd

			lda samp4
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

			.byte ( 0*80) & 0xff
			.byte ( 1*80) & 0xff
			.byte ( 2*80) & 0xff
			.byte ( 3*80) & 0xff
			.byte ( 4*80) & 0xff
			.byte ( 5*80) & 0xff
			.byte ( 6*80) & 0xff
			.byte ( 7*80) & 0xff
			.byte ( 8*80) & 0xff
			.byte ( 9*80) & 0xff
			.byte (10*80) & 0xff
			.byte (11*80) & 0xff
			.byte (12*80) & 0xff
			.byte (13*80) & 0xff
			.byte (14*80) & 0xff
			.byte (15*80) & 0xff
			.byte (16*80) & 0xff
			.byte (17*80) & 0xff
			.byte (18*80) & 0xff
			.byte (19*80) & 0xff
			.byte (20*80) & 0xff
			.byte (21*80) & 0xff
			.byte (22*80) & 0xff
			.byte (23*80) & 0xff
			.byte (24*80) & 0xff

			.public times80tablehi
times80tablehi:

			.byte (( 0*80) >> 8) & 0xff
			.byte (( 1*80) >> 8) & 0xff
			.byte (( 2*80) >> 8) & 0xff
			.byte (( 3*80) >> 8) & 0xff
			.byte (( 4*80) >> 8) & 0xff
			.byte (( 5*80) >> 8) & 0xff
			.byte (( 6*80) >> 8) & 0xff
			.byte (( 7*80) >> 8) & 0xff
			.byte (( 8*80) >> 8) & 0xff
			.byte (( 9*80) >> 8) & 0xff
			.byte ((10*80) >> 8) & 0xff
			.byte ((11*80) >> 8) & 0xff
			.byte ((12*80) >> 8) & 0xff
			.byte ((13*80) >> 8) & 0xff
			.byte ((14*80) >> 8) & 0xff
			.byte ((15*80) >> 8) & 0xff
			.byte ((16*80) >> 8) & 0xff
			.byte ((17*80) >> 8) & 0xff
			.byte ((18*80) >> 8) & 0xff
			.byte ((19*80) >> 8) & 0xff
			.byte ((20*80) >> 8) & 0xff
			.byte ((21*80) >> 8) & 0xff
			.byte ((22*80) >> 8) & 0xff
			.byte ((23*80) >> 8) & 0xff
			.byte ((24*80) >> 8) & 0xff
