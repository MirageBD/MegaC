#ifndef __FONTSYS_H
#define __FONTSYS_H

extern uint8_t fontsys_asciiremap[];

void fontsys_init();
void fontsys_map();
void fontsys_unmap();
void fontsys_test();

extern void fontsys_asm_init();
extern void fontsys_asm_test();

extern uint8_t fnts_screentablo;
extern uint8_t fnts_screentabhi;
extern uint8_t fnts_attribtablo;
extern uint8_t fnts_attribtabhi;

extern uint8_t fnts_readchar;

extern uint8_t fnts_row;
extern uint8_t fnts_column;
extern uint8_t fnts_curpal;

#endif