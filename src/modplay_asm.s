				.extern modplay_play_c
				.extern ticks
				.extern mod_speed

				; structtest:		.space 2

; ------------------------------------------------------------------------------------

			.public modplay_play
modplay_play:

			dec ticks
			beq fetchnextrow

			; perform effects for this tick
			bra exit

fetchnextrow:

			lda mod_speed
			sta ticks
			jsr modplay_play_c

exit:		rts

; ------------------------------------------------------------------------------------
