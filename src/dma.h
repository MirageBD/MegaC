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

typedef struct _dma_copyjob
{
	uint8_t		sourcemb_token;
	uint8_t		sourcemb;
	uint8_t		destmb_token;
	uint8_t		destmb;
	uint8_t		format;
	uint8_t		endtokenlist;
	uint8_t		command_lo;
	uint16_t	count;
	uint16_t	source_addr;
	uint8_t		source_bank_and_flags;
	uint16_t	dest_addr;
	uint8_t		dest_bank_and_flags;
	uint8_t		command_hi;
	uint16_t	modulo_and_mode;
} dma_copyjob;

void dma_runjob(__far char *ptr);
void dma_dmacopy(uint32_t source_address, uint32_t destination_address, uint16_t count);
void dma_init();

#endif
