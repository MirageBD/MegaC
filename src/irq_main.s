			.extern modplay_play
			.extern keyboard_update
			.extern keyboard_test
			.extern fontsys_clearscreen
			.extern program_update

; ------------------------------------------------------------------------------------

			.public irq_main
irq_main:
			php
			pha
			phx
			phy
			phz

			lda #0x06
			sta 0xd020
			jsr modplay_play

			lda #0x0f
			sta 0xd020

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

			lda #0x08
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