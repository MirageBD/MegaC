#ifndef _SDC_H
#define _SDC_H

extern void			sdc_opendir();
extern void			sdc_asm_opendir();

extern void			sdc_chdir();
extern void			sdc_asm_chdir();

extern void			sdc_hyppoopenfile();
extern void			sdc_asm_hyppoopenfile();

extern void			sdc_openfile();
extern void			sdc_asm_openfile();

extern void			sdc_hyppoclosefile();
extern void			sdc_asm_hyppoclosefile();

extern void			sdc_readfirstsector();
extern void			sdc_asm_readfirstsector();

extern void			sdc_geterror();
extern void			sdc_asm_geterror();

extern void			sdc_processdirentry();
extern void			sdc_setbufferaddressmsb(uint8_t msb);
extern void			sdc_setprocessdirentryfunc(uint16_t funcptr);

extern uint8_t		sdc_transferbuffermsb;
extern uint8_t*		sdc_processdirentryptr;

extern uint8_t		sdc_loadaddresslo;
extern uint8_t		sdc_loadaddressmid;
extern uint8_t		sdc_loadaddresshi;

extern uint8_t		sdc_asm_filedescriptor;

#endif