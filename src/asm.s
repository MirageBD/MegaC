				.extern modplay_play
				.extern tempo
				.extern ticks
				.extern speed

/*
				.extern foo

FOO_A			.equ 0
FOO_B			.equ 1

				.public asm_testfoo
asm_testfoo:	lda #0xba
				sta foo+FOO_A
				lda #0xbe
				sta foo+FOO_B
				rts
*/

; ------------------------------------------------------------------------------------

			.public irqvblank
irqvblank:
			php
			pha
			phx
			phy
			phz

			; save state

			dec ticks
			beq fetchnextrow

			; perform effects for this tick
			bra exit

fetchnextrow:

			lda speed
			sta ticks

			lda #0x01
			sta 0xd020
			jsr modplay_play
			lda #0x00
			sta 0xd020

exit:		; restore state

			plz
			ply
			plx
			pla
			plp
			asl 0xd019
			rti

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

