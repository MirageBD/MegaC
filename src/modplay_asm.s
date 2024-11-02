				.extern modplay_play_c

				.public tempo
tempo:			.byte 0
				.public ticks
ticks:			.byte 0
				.public speed
speed:			.byte 0 ; speed gets fed into ticks. ticks decreases every frame until 0, which triggers a row increase.

				; structtest:		.space 2

; ------------------------------------------------------------------------------------

			.public modplay_play
modplay_play:

			; save state
			dec ticks
			beq fetchnextrow

			; perform effects for this tick
			bra exit

fetchnextrow:
			lda speed
			sta ticks
			jsr modplay_play_c

exit:		rts

; ------------------------------------------------------------------------------------
