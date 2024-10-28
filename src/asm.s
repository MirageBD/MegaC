			.public setborder
setborder:	sta 0xd020
			rts

			.public irq1
irq1:		pha
			inc 0xd020
			pla
			asl 0xd019
			rti