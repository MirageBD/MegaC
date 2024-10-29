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

			inc foo
			lda foo
			cmp #0x04
			bne bar
			lda 0
			sta foo

			jsr modplay_play
			inc 0xd021

bar			plz
			ply
			plx
			pla
			plp
			asl 0xd019
			rti

foo			.byte 0

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