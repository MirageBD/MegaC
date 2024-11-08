#ifndef __FONTSYS_H
#define __FONTSYS_H

void fontsys_init();
void fontsys_test();

extern void fontsys_asm_init();
extern void fontsys_asm_test();

extern uint8_t fnts_screentablo;
extern uint8_t fnts_screentabhi;
extern uint8_t fnts_attribtablo;
extern uint8_t fnts_attribtabhi;

#endif