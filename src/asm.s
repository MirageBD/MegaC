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
			cmp #0x07
			bne next$
			lda #0x00
			sta timer

			inc 0xd020
			jsr modplay_play
			dec 0xd020
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