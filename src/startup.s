		.rtmodel cstartup, "mystartup"

		.rtmodel version, "1"
		.rtmodel cpu, "*"

		.section data_init_table
		.section stack
		.section cstack
		.section heap

		.extern main
		.extern _Zp, _Vsp

;;; *** Need to manually provide start address as we use raw binary output format and not prg.
		.pubweak __program_root_section, __program_start

		.section programStart, root
__program_root_section:

__program_start:

		sei
		jmp __data_initialization_needed

;;; Initialize data sections.
		.section startup, noroot, noreorder
		.pubweak __data_initialization_needed
__data_initialization_needed:
		lda #.byte0 (.sectionStart data_init_table)
		sta zp:_Zp
		lda #.byte1 (.sectionStart data_init_table)
		sta zp:_Zp+1
		lda #.byte0 (.sectionEnd data_init_table)
		sta zp:_Zp+2
		lda #.byte1 (.sectionEnd data_init_table)
		sta zp:_Zp+3
		.extern __initialize_sections
		jsr __initialize_sections

		jmp __call_heap_initialize

;;; **** Initialize heap.
		.section startup, noroot, noreorder
		.pubweak __call_heap_initialize
__call_heap_initialize:
		lda #.byte0 __default_heap
		sta zp:_Zp+0
		lda #.byte1 __default_heap
		sta zp:_Zp+1
		lda #.byte0 (.sectionStart heap)
		sta zp:_Zp+2
		lda #.byte1 (.sectionStart heap)
		sta zp:_Zp+3
		lda #.byte0 (.sectionSize heap)
		sta zp:_Zp+4
		lda #.byte1 (.sectionSize heap)
		sta zp:_Zp+5

		.extern __heap_initialize, __default_heap
		jsr __heap_initialize

;flashloop$:
;		inc 0xd020
;		jmp flashloop$

		lda #.byte0(nmi_handler)
		sta 0xfffa
		lda #.byte1(nmi_handler)
		sta 0xfffb

;;; Jump into C main function.
		jmp main

;;; Disable NMI
		.section code
nmi_handler:
		rti
