			.extern modplay_play

; ------------------------------------------------------------------------------------

			.public setborder
setborder:	sta 0xd020
			rts

; ------------------------------------------------------------------------------------

			.public irq1
irq1:
			php
			pha
			phx
			phy
			phz

			inc timer
			lda timer
			cmp #0x04
			bne next$
			lda #0x00
			sta timer

			jsr modplay_play
			inc 0xd021

next$:		plz
			ply
			plx
			pla
			plp
			asl 0xd019
			rti

timer		.byte 0

; ------------------------------------------------------------------------------------

			.public fastload_irq
fastload_irq:
			php
			pha
			phx
			phy

			inc 0xd020

			ply
			plx
			pla
			plp
			asl 0xd019
			rti

; ------------------------------------------------------------------------------------