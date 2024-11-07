#include "dma.h"
#include "registers.h"
#include "macros.h"

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
	poke(0xd702U, 0);
	poke(0xd704U, 0);

	poke(0xd701U,((unsigned int)&dmalist)>>8);
	poke(0xd700U,((unsigned int)&dmalist)&0xff); // triggers DMA
}

void dma_lcopy(long source_address, long destination_address, unsigned int count)
{
	dmalist.command			= 0x00; // copy
	dmalist.count			= count;
	dmalist.source_addr		= (source_address & 0xffff);
	dmalist.source_bank		= (source_address >> 16) & 0x7f;
	dmalist.dest_addr		= (destination_address & 0xffff);
	dmalist.dest_bank		= (destination_address >> 16) & 0x7f;

	do_dma();

	return;
}

void dma_lfill(long destination_address, unsigned char value, unsigned int count)
{
	dmalist.command     = 0x03; // fill
	dmalist.count       = count;
	dmalist.source_addr = value;
	dmalist.source_bank = 0;
	dmalist.dest_addr   = (destination_address & 0xffff);
	dmalist.dest_bank   = (destination_address >> 16) & 0x7f;

	do_dma();

	return;
}
