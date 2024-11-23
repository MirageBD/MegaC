			.rtmodel cpu, "*"
			
			.extern modplay_play
			.extern keyboard_update
			.extern keyboard_test
			.extern fontsys_clearscreen
			.extern program_update
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

; ------------------------------------------------------------------------------------

			.public irq_main
irq_main:
			php
			pha
			phx
			phy
			phz

			lda #0x0f
			sta 0xd020
			sta 0xd021

			jsr modplay_play
			jsr fontsys_clearscreen
			jsr program_update
			jsr keyboard_update

			lda #0x32 + 12*8 + 1
			sta 0xd012
			lda #.byte0 irq_main2
			sta 0xfffe
			lda #.byte1 irq_main2
			sta 0xffff

			plz
			ply
			plx
			pla
			plp
			asl 0xd019
			rti

; ------------------------------------------------------------------------------------

irq_main2:
			php
			pha
			phx
			phy
			phz

			lda 0xd012
waitr1$:	cmp 0xd012
			beq waitr1$

			clc
			lda 0xd012
			adc #0x08

			ldx #0xe2
			stx 0xd20f
			stx 0xd21f
			ldx #0xf4
			stx 0xd30f
			stx 0xd31f

waitr2$:	cmp 0xd012
			bne waitr2$

			ldx #0x00
			stx 0xd20f
			stx 0xd21f
			stx 0xd30f
			stx 0xd31f

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

			lda samp1max
			asl a
			asl a
			jsr swapnybbles
			sta 0xd10f

			ldy #0x08
			ldx #0x00
a1$:		lda 0xd020
			dex
			bne a1$
			dey
			bne a1$

			lda samp2max
			asl a
			asl a
			jsr swapnybbles
			sta 0xd10f

			ldy #0x08
			ldx #0x00
a2$:		lda 0xd020
			dex
			bne a2$
			dey
			bne a2$

			lda samp3max
			asl a
			asl a
			jsr swapnybbles
			sta 0xd10f

			ldy #0x08
			ldx #0x00
a3$:		lda 0xd020
			dex
			bne a3$
			dey
			bne a3$

			lda samp4max
			asl a
			asl a
			jsr swapnybbles
			sta 0xd10f

			ldy #0x08
			ldx #0x00
a4$:		lda 0xd020
			dex
			bne a4$
			dey
			bne a4$

			lda #0x00
			sta 0xd10f
			sta 0xd20f
			sta 0xd30f

			lda #0xff
			sta 0xd012
			lda #.byte0 irq_main
			sta 0xfffe
			lda #.byte1 irq_main
			sta 0xffff

			plz
			ply
			plx
			pla
			plp
			asl 0xd019
			rti

; ------------------------------------------------------------------------------------

swapnybbles:

			asl a						; swap nybbles
			adc #0x80
			rol a
			asl a
			adc #0x80
			rol a
			rts

; ------------------------------------------------------------------------------------

			; these are just shifted so can be combined!

spectrumpalred:
			.byte 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			.byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			

spectrumpalgreen:
			.byte 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			.byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

spectrumpalblue:
			.byte 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			.byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
