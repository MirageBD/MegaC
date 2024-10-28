			.public setborder
setborder:	sta 0xd020
			rts

			.public irq1
irq1:
			php
			pha
			phx
			phy

			inc 0xd021

			ply
			plx
			pla
			plp
			asl 0xd019
			rti

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