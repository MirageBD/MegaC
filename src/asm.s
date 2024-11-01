			.extern modplay_play
			.extern modplay_increment
			.extern tempo

; ------------------------------------------------------------------------------------

			.public setborder
setborder:	sta 0xd020
			rts

; ------------------------------------------------------------------------------------

startraster:		.byte 0
playtimetaken:		.byte 0
compensatedtempo:	.byte 0

			.public irqcia
irqcia:
			php
			pha
			phx
			phy
			phz

playloop:
			lda 0xd012
			sta startraster

			lda #0x01
			sta 0xd020

			jsr modplay_play

			lda #0x00
			sta 0xd020

			sec
			lda 0xd012
			sbc startraster
			sta playtimetaken

			sec
			lda tempo+0
			sbc playtimetaken
			sta compensatedtempo+0
			lda tempo+1
			sbc #0x00
			sta compensatedtempo+1

			ldy compensatedtempo+1
			ldx compensatedtempo+0
waitx:		lda 0xd012
waitr:		cmp 0xd012
			beq waitr
			;inc 0xd020
			dex
			bne waitx
			dey
			bpl waitx

			jmp playloop

			plz
			ply
			plx
			pla
			plp
			asl 0xd019
			rti

; ------------------------------------------------------------------------------------

			.public irqvblank
irqvblank:
			php
			pha
			phx
			phy
			phz

			sec
			lda tempodec+0
			sbc #0x70						; decrease with <(2 * 312)
			sta tempodec+0
			lda tempodec+1
			sbc #0x02						; decrease with >(2 * 312)
			sta tempodec+1
			bcs next$

			lda #0x01
			sta 0xd020
			jsr modplay_play
			lda tempo+0
			sta tempodec+0
			lda tempo+1
			sta tempodec+1
			lda #0x00
			sta 0xd020

next$:		plz
			ply
			plx
			pla
			plp
			asl 0xd019
			rti

frametimer		.byte 0
tempodec		.word 0x0ea0

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