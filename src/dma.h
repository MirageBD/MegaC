#ifndef __DMA_H
#define __DMA_H

typedef struct _dma_job                 dma_job_t;
typedef struct _dma_job_far_source      dma_job_far_source_t;
typedef struct _dma_job_far_destination dma_job_far_destination_t;
typedef struct _dma_job_far_addresses   dma_job_far_addresses_t;

struct _dma_job
{
	char			type;
	char			end_options;
	char			command;
	unsigned short	count;
	unsigned short	source;
	char			source_bank;
	unsigned short	destination;
	char			destination_bank;
	unsigned short	modulo;
};

struct _dma_job_far_source
{
	char			type;
	char			sbank_token;
	char			sbank;
	char			end_options;
	char			command;
	unsigned short	count;
	unsigned short	source;
	char			source_bank;
	unsigned short	destination;
	char			destination_bank;
	unsigned short	modulo;
};

struct _dma_job_far_destination
{
	char			type;
	char			dbank_token;
	char			dbank;
	char			end_options;
	char			command;
	unsigned short	count;
	unsigned short	source;
	char			source_bank;
	unsigned short	destination;
	char			destination_bank;
	unsigned short	modulo;
};

struct _dma_job_far_addresses
{
	char			type;
	char			sbank_token;
	char			sbank;
	char			dbank_token;
	char			dbank;
	char			end_options;
	char			command;
	unsigned short	count;
	unsigned short	source;
	char			source_bank;
	unsigned short	destination;
	char			destination_bank;
	unsigned short	modulo;
};

extern void run_dma_job(__far char *ptr);
extern void dma_lcopy(long source_address, long destination_address, unsigned int count);

#endif
