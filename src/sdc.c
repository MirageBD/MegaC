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

void sdc_hyppo_loadfile()
{
	sdc_asm_hyppo_loadfile();
}

void sdc_hyppo_loadfile_attic()
{
	sdc_asm_hyppo_loadfile_attic();
}

void sdc_openfile()
{
	sdc_asm_openfile();
}

void sdc_readfirstsector()
{
	sdc_asm_readfirstsector();
}

void sdc_hyppoclosefile()
{
	sdc_asm_hyppoclosefile();
}

void sdc_geterror()
{
	sdc_asm_geterror();
}

void sdc_chunk_readasync()
{
	sdc_asm_chunk_readasync();
}
