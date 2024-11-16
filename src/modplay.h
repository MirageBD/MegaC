#ifndef _MODPLAY_H
#define _MODPLAY_H

extern void modplay_init();
extern void modplay_initmod(uint32_t attic_address, uint32_t sample_address);
extern void modplay_mute();
extern uint8_t mp_enabled_channels[4];

extern void modplay_enable();
extern void modplay_disable();

extern uint8_t mp_loop;
extern uint8_t mp_done;

#endif