			.extern modplay_play

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

			plz
			ply
			plx
			pla
			plp
			asl 0xd019
			rti

highestraster	.byte 0x00

; ------------------------------------------------------------------------------------