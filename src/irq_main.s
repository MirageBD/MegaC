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

			lda #0x0a
			sta 0xd020
			jsr fontsys_clearscreen

			lda #0x02
			sta 0xd020
			jsr keyboard_update
			jsr program_update

			lda #0x0f
			sta 0xd020

			plz
			ply
			plx
			pla
			plp
			asl 0xd019
			rti

; ------------------------------------------------------------------------------------
