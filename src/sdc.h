#ifndef _SDC_H
#define _SDC_H

extern void sdc_processdirentry();
extern void sdc_setbufferaddressmsb(uint8_t msb);
extern void sdc_setprocessdirentryptr(uint16_t ptr);

extern uint8_t sdc_transferbuffermsb;
extern uint8_t* sdc_processdirentryptr;

#endif