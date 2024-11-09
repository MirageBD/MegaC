#include <stdint.h>
#include "sdc.h"
#include "macros.h"
#include "constants.h"

void sdc_setbufferaddressmsb(uint8_t msb)
{
	poke(&sdc_transferbuffermsb, msb);
}

void sdc_setprocessdirentryfunc(uint16_t funcptr)
{
	poke(((uint8_t *)&sdc_processdirentryptr + 1), (uint8_t)((funcptr >> 0) & 0xff));
	poke(((uint8_t *)&sdc_processdirentryptr + 2), (uint8_t)((funcptr >> 8) & 0xff));
}

void sdc_opendir()
{
	sdc_asm_opendir();
}

void sdc_chdir()
{
	sdc_asm_chdir();
}

void sdc_openfile()
{
	sdc_asm_openfile();
}
