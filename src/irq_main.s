			;.rtmodel version,"1"
			;.rtmodel codeModel,"plain"
			;.rtmodel core,"45gs02"
			;.rtmodel target,"mega65"
			
			.extern modplay_play
			.extern keyboard_update
			.extern keyboard_test
			.extern fontsys_clearscreen
			.extern program_update
			.extern _Zp

zp_cadr		.equlab	_Zp + 40

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
			stx 0xd22f
			ldx #0xf4
			stx 0xd30f
			stx 0xd32f

waitr2$:	cmp 0xd012
			bne waitr2$

			ldx #0x00
			stx 0xd20f
			stx 0xd22f
			stx 0xd30f
			stx 0xd32f


			ldx #0x00

loopsamplerend$:
			lda 0xd72a
			sta zp_cadr+0
			lda 0xd72b
			sta zp_cadr+1
			lda 0xd72c
			sta zp_cadr+2
			lda #0x00
			sta zp_cadr+3

			ldz #0x00
			lda [zp:zp_cadr],z
			lsr a

			asl a						; swap nybbles
			adc #0x80
			rol a
			asl a
			adc #0x80
			rol a

			sta 0xd10f

			lda 0xd73a
			sta zp_cadr+0
			lda 0xd73b
			sta zp_cadr+1
			lda 0xd73c
			sta zp_cadr+2
			lda #0x00
			sta zp_cadr+3

			ldz #0x00
			lda [zp:zp_cadr],z
			lsr a

			asl a						; swap nybbles
			adc #0x80
			rol a
			asl a
			adc #0x80
			rol a

			sta 0xd20f

			lda 0xd74a
			sta zp_cadr+0
			lda 0xd74b
			sta zp_cadr+1
			lda 0xd74c
			sta zp_cadr+2
			lda #0x00
			sta zp_cadr+3

			ldz #0x00
			lda [zp:zp_cadr],z
			lsr a

			asl a						; swap nybbles
			adc #0x80
			rol a
			asl a
			adc #0x80
			rol a

			sta 0xd30f

			lda 0xd012
waitr4$:	cmp 0xd012
			beq waitr4$

			inx
			cpx #50
			beq loopsamplerendend$
			jmp loopsamplerend$

loopsamplerendend$:

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
