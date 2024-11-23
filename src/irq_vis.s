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

; ------------------------------------------------------------------------------------

			.public irq_vis
irq_vis:
			php
			pha
			phx
			phy
			phz

			jsr modplay_play
			jsr fontsys_clearscreen
			jsr program_update_vis
			jsr keyboard_update

			lda #0x32 + 6*8 + 1
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
			ldx channel_sample+0
			jsr rendercolour
			jsr wait8lines

			lda samp2max
			ldx channel_sample+1
			jsr rendercolour
			jsr wait8lines

			lda samp3max
			ldx channel_sample+2
			jsr rendercolour
			jsr wait8lines

			lda samp4max
			ldx channel_sample+3
			jsr rendercolour
			jsr wait8lines

			lda #0x00
			sta 0xd10f
			sta 0xd20f
			sta 0xd30f

			lda #0xff
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

wait8lines:
			ldy #0x10
			ldx #0x00
a1$:		lda 0xd020
			dex
			bne a1$
			dey
			bne a1$
			rts

; ------------------------------------------------------------------------------------

rendercolour:
			; put current volume in first MULT register
			asl a
			asl a
			sta 0xd770

			; and put red, green and blue in secondary MULT register. take output and colourize
			lda spectrumpalred,x
			sta 0xd774
			lda 0xd779
			jsr swapnybbles
			sta 0xd10f

			lda spectrumpalgreen,x
			sta 0xd774
			lda 0xd779
			jsr swapnybbles
			sta 0xd20f

			lda spectrumpalblue,x
			sta 0xd774
			lda 0xd779
			jsr swapnybbles
			sta 0xd30f

			rts

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

spectrumpalred:
			.byte 0xff, 0x00, 0x40, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff
			.byte 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff

spectrumpalgreen:
			.byte 0x80, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00
			.byte 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff

spectrumpalblue:
			.byte 0x00, 0x80, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00
			.byte 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00
