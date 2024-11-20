(define memories
'(
	(memory zeroPage (address (#x2 . #x7f)) (type ram) (qualifier zpage)
		(section (registers #x2))
	)
	
	(memory stackPage (address (#x100 . #x1ff))
		(type ram)
	)

	(memory storage0 (address (#x0a00 . #x0fff)) (type any)
		(section
			(data0a00 #x0a00)
		)
	)

	(memory prog (address (#x0a00 . #x6fff)) (type any)
		(section
			(programStart #x1000)
			startup
			code
			data
			switch
			cdata
			data_init_table
		)
	)

	(memory hzd (address (#x7000 . #x7fff)) (type any)
		(section
			heap
			zdata
		)
	)

	(memory storage1 (address (#xe000 . #xffff)) (type any)
		(section
			(data10000 #xe000)
		)
	)
))
