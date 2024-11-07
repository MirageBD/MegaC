#ifndef __MACROS_H
#define __MACROS_H

#define poke(a, t)		*((volatile       char *)(a)) = t
#define lpoke(a, t)		*((volatile __far char *)(a)) = t
#define peek(a)			*((volatile       char *)(a))
#define lpeek(a)		*((volatile __far char *)(a))
#define set_bit(a, t)	*((               char *)(a)) |= t
#define clear_bit(a, t)	*((               char *)(a)) &= ~t

typedef void            (*PROC_PTR)(void);

#define UNMAP_ALL       __asm(" lda #0x00\n" " tax\n" " tay\n" " taz\n" " map\n" " nop");

#define SEI             __asm(" sei");
#define CLI             __asm(" cli");

#endif
