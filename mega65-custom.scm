(define memories
'(
	(memory zeroPage (address (#x2 . #x7f)) (type ram) (qualifier zpage)
		(section (registers #x2))
	)
	
	(memory stackPage (address (#x100 . #x1ff))
		(type ram)
	)

	(memory prog (address (#x0a00 . #xdfff)) (type any)
		(section
			(data0a00 #x0a00)
			(programStart #x1000)
			startup
			code
			data
			switch
			cdata
			data_init_table
		)
	)

	(memory himem (address (#xe000 . #xffff)) (type any)
		(section
			(data10000 #xe000)
		)
	)
))
