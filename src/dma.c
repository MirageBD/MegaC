#include "dma.h"
#include "chips.h"

void run_dma_job(__far char *ptr)
{
	DMA.ADDRMB   = 0;			  
	DMA.ADDRBANK = (char)((unsigned long)ptr >> 16);
	DMA.ADDRMSB  = (char)((unsigned long)ptr >> 8);
	DMA.ETRIG    = (char)((unsigned long)ptr & 0xff);
}
