#ifndef __DMA_H
#define __DMA_H

#include <stdint.h>

typedef struct _dma_job
{
	uint8_t		type;
	uint8_t		sbank_token;
	uint8_t		sbank;
	uint8_t		dbank_token;
	uint8_t		dbank;
	uint8_t		dskipratefrac_token;
	uint8_t		dskipratefrac;
	uint8_t		dskiprate_token;
	uint8_t		dskiprate;
	uint8_t		end_options;
	uint8_t		command;
	uint16_t	count;
	uint16_t	source;
	uint8_t		source_bank;
	uint16_t	destination;
	uint8_t		destination_bank;
	uint16_t	modulo;
} dma_job;

void run_dma_job(__far char *ptr);

#endif
