			.extern modplay_play
			.extern process_input
			.extern keyboard_pressed
			.extern keyboard_toascii

; ------------------------------------------------------------------------------------

			.public irq_main
irq_main:
			php
			pha
			phx
			phy
			phz

			lda #0x01
			sta 0xd020
			jsr modplay_play
			lda #0x00
			sta 0xd020

			lda 0xd012
			cmp highestraster
			bmi irq_main_end
			sta highestraster

irq_main_end:
			lda highestraster
raswait:	cmp 0xd012
			bne raswait

			lda #0x02
			sta 0xd020
			ldx 0xd012
			inx
raswait2:	cpx 0xd012
			bne raswait2
			lda #0x00
			sta 0xd020

			jsr process_input

			ldx keyboard_pressed
			lda keyboard_toascii,x
			sta 0xd020

			;cmp #0x01					; is it 'a'?
			;bne irq_end
			;lda #0x01
			;sta 0xd020

irq_end		plz
			ply
			plx
			pla
			plp
			asl 0xd019
			rti

highestraster	.byte 0x00

; ------------------------------------------------------------------------------------
