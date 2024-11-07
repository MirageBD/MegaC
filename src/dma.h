#ifndef __DMA_H
#define __DMA_H

extern void run_dma_job(__far char *ptr);
extern void dma_lcopy(long source_address, long destination_address, unsigned int count);
extern void dma_lfill(long destination_address, unsigned char value, unsigned int count);

#endif
