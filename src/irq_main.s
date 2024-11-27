			.rtmodel cpu, "*"
			
			.extern keyboard_update
			.extern fontsys_clearscreen
			.extern program_update
			.extern _Zp

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

			jsr fontsys_clearscreen
			jsr keyboard_update
			jsr program_update

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

			lda #0xff
			sta 0xd012
			.public irqvec0
irqvec0:
			lda #.byte0 irq_main
			sta 0xfffe
			.public irqvec1
irqvec1:
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
