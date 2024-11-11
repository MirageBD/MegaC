#include "dma.h"
#include "registers.h"

dma_copyjob dma_global_copyjob;

void dma_runjob(__far char *ptr)
{
	DMA.ADDRMB   = 0;
	DMA.ADDRBANK = (char)((unsigned long)ptr >> 16);
	DMA.ADDRMSB  = (char)((unsigned long)ptr >> 8);
	DMA.ETRIG    = (char)((unsigned long)ptr & 0xff);
}

void dma_dmacopy(uint32_t source_address, uint32_t destination_address, uint16_t count)
{
	dma_global_copyjob.count					= count;
	dma_global_copyjob.source_addr				= (source_address           ) & 0xffff;
	dma_global_copyjob.source_bank_and_flags	= (source_address      >> 16) & 0x7f;
	dma_global_copyjob.sourcemb					= (source_address      >> 20) & 0xff;
	dma_global_copyjob.dest_addr				= (destination_address      ) & 0xffff;
	dma_global_copyjob.dest_bank_and_flags		= (destination_address >> 16) & 0x7f;
	dma_global_copyjob.destmb					= (destination_address >> 20) & 0xff;

	DMA.EN018B				= 0x01;
	DMA.ADDRBANK			= 0x00;
	DMA.ADDRMSB				= (((uint16_t)&dma_global_copyjob) >> 8);
	DMA.ETRIG				= (((uint16_t)&dma_global_copyjob) & 0xff);
}

void dma_init()
{
	dma_global_copyjob.command_lo		= 0x00; // copy
	dma_global_copyjob.sourcemb_token	= 0x80;
	dma_global_copyjob.destmb_token		= 0x81;
	dma_global_copyjob.format			= 0x0b;
	dma_global_copyjob.endtokenlist		= 0x00;
	dma_global_copyjob.command_hi		= 0x00;
}