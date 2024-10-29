#include "dma.h"
#include "registers.h"
#include "macros.h"

void run_dma_job(__far char *ptr)
{
	DMA.ADDRMB   = 0;			  
	DMA.ADDRBANK = (char)((unsigned long)ptr >> 16);
	DMA.ADDRMSB  = (char)((unsigned long)ptr >> 8);
	DMA.ETRIG    = (char)((unsigned long)ptr & 0xff);
}







// stuff originally added for lcopy

struct dmagic_dmalist
{
	unsigned char command;
	unsigned int  count;
	unsigned int  source_addr;
	unsigned char source_bank;
	unsigned int  dest_addr;
	unsigned char dest_bank;
	unsigned int  modulo;
};

struct dmagic_dmalist dmalist;

void do_dma(void)
{
    /*
    .macro DMA_RUN_JOB jobPointer
            lda #(jobPointer & $ff0000) >> 16
            sta $d702
            sta $d704
            lda #>jobPointer
            sta $d701
            lda #<jobPointer
            sta $d705
    .endmacro
    */

	// Gate C65 IO enable
	// poke(0xd02fU,0xa5);
	// poke(0xd02fU,0x96);
	// Force back to 3.5MHz
	// POKE(0xD031,PEEK(0xD031)|0x40);

	poke(0xd702U, 0);
	poke(0xd704U, 0);
	// everything OK here
	//poke(0xd705U, 0);
	// everything not OK here
	//poke(0xd706U, 0);

	poke(0xd701U,((unsigned int)&dmalist)>>8);
	poke(0xd700U,((unsigned int)&dmalist)&0xff); // triggers DMA
}

// this was originally lcopy
// LV TODO - CLEAN THIS UP, IT'S A MESS!!!
void dma_lcopy(long source_address, long destination_address, unsigned int count)
{
	dmalist.command     = 0x00; // copy
	dmalist.count       = count;
	dmalist.source_addr = source_address & 0xffff;
	dmalist.source_bank = (source_address >> 16) & 0x7f;
	dmalist.dest_addr   = destination_address & 0xffff;
	dmalist.dest_bank   = (destination_address >> 16) & 0x7f;

	do_dma();

	return;
}
