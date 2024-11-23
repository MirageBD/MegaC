#ifndef __PROGRAM_H
#define __PROGRAM_H

void program_loaddata();
void program_init();
void program_mainloop();
void program_update();
void program_update_vis();

extern uint8_t irqvec0;
extern uint8_t irqvec1;
extern uint8_t irqvec2;
extern uint8_t irqvec3;

#endif