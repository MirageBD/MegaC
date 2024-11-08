			.extern modplay_play
			.extern keyboard_update
			.extern keyboard_test
			.extern fontsys_clearscreen
			.extern fontsys_test

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

			lda 0xd012
			cmp highestraster
			bmi irq_main_end
			sta highestraster

irq_main_end:
			lda highestraster
raswait:	cmp 0xd012
			bne raswait

			lda #0x0e
			sta 0xd020
			ldx 0xd012
			inx
raswait2:	cpx 0xd012
			bne raswait2
			lda #0x13
			sta 0xd020

			jsr keyboard_update
			jsr keyboard_test

			lda #0x01
			sta 0xd020

			jsr fontsys_clearscreen

			lda #0x03
			sta 0xd020

			jsr fontsys_test

			lda #0x00
			sta 0xd020

			plz
			ply
			plx
			pla
			plp
			asl 0xd019
			rti

highestraster	.byte 0x00

; ------------------------------------------------------------------------------------
