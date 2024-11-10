#ifndef _MODPLAY_H
#define _MODPLAY_H

extern void modplay_init(uint32_t address, uint32_t attic_address);
extern uint8_t mp_enabled_channels[4];

extern void modplay_enable();
extern void modplay_disable();

#endif