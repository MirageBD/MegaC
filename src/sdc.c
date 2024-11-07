#include <stdint.h>
#include "sdc.h"
#include "macros.h"

void sdc_setbufferaddressmsb(uint8_t msb)
{
	poke(&sdc_transferbuffermsb, msb);
}

void sdc_setprocessdirentryptr(uint16_t ptr)
{
	poke(((uint8_t *)&sdc_processdirentryptr + 1), (uint8_t)((ptr >> 0) & 0xff));
	poke(((uint8_t *)&sdc_processdirentryptr + 2), (uint8_t)((ptr >> 8) & 0xff));
}

void sdc_processdirentry()
{
	uint8_t* transbuf = (uint8_t *)(sdc_transferbuffermsb * 256);

	for(int i=0; i<64; i++)
		poke(0xc000+i, transbuf[i]);
}