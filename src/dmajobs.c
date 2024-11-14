#include "constants.h"
#include "dmajobs.h"

dma_job dma_clearcolorram1 =
{
	.type					= 0x0a,
	.sbank_token			= 0x80,
	.sbank					= 0x00,
	.dbank_token			= 0x81,
	.dskipratefrac_token	= 0x84,
	.dskipratefrac			= 0x00,
	.dskiprate_token		= 0x85,
	.dskiprate				= 0x02,
	.dbank					= ((SAFE_COLOR_RAM + 0) >> 20),
	.end_options			= 0x00,
	.command				= 0b00000011, // fill, no chain
	.count					= (RRBSCREENWIDTH*50),
	.source					= 0b0000000000001100, // 00001000 = NCM chars, trim 8 pixels
	.source_bank			= 0x00,
	.destination			= ((SAFE_COLOR_RAM + 0) & 0xffff),
	.destination_bank		= (((SAFE_COLOR_RAM + 0) >> 16) & 0x0f),
	.modulo					= 0x0000
};

dma_job dma_clearcolorram2 =
{
	.type					= 0x0a,
	.sbank_token			= 0x80,
	.sbank					= 0x00,
	.dbank_token			= 0x81,
	.dskipratefrac_token	= 0x84,
	.dskipratefrac			= 0x00,
	.dskiprate_token		= 0x85,
	.dskiprate				= 0x02,
	.dbank					= ((SAFE_COLOR_RAM + 1) >> 20),
	.end_options			= 0x00,
	.command				= 0b00000011, // fill, no chain
	.count					= (RRBSCREENWIDTH*50),
	.source					= 0b0000000000001111, // 00000000 = $0f = pixels with value $0f take on the colour value of $0f as well
	.source_bank			= 0x00,
	.destination			= ((SAFE_COLOR_RAM + 1) & 0xffff),
	.destination_bank		= (((SAFE_COLOR_RAM + 1) >> 16) & 0x0f),
	.modulo					= 0x0000
};

dma_job dma_clearscreen1 =
{
	.type					= 0x0a,
	.sbank_token			= 0x80,
	.sbank					= 0x00,
	.dbank_token			= 0x81,
	.dskipratefrac_token	= 0x84,
	.dskipratefrac			= 0x00,
	.dskiprate_token		= 0x85,
	.dskiprate				= 0x02,
	.dbank					= ((SCREEN + 0) >> 20),
	.end_options			= 0x00,
	.command				= 0b00000011, // fill, no chain
	.count					= (RRBSCREENWIDTH*50),
	.source					= (((FONTCHARMEM/64 + 0 /*star=10*/) >> 0)) & 0xff,
	.source_bank			= 0x00,
	.destination			= ((SCREEN + 0) & 0xffff),
	.destination_bank		= (((SCREEN + 0) >> 16) & 0x0f),
	.modulo					= 0x0000
};

dma_job dma_clearscreen2 =
{
	.type					= 0x0a,
	.sbank_token			= 0x80,
	.sbank					= 0x00,
	.dbank_token			= 0x81,
	.dskipratefrac_token	= 0x84,
	.dskipratefrac			= 0x00,
	.dskiprate_token		= 0x85,
	.dskiprate				= 0x02,
	.dbank					= ((SCREEN + 1) >> 20),
	.end_options			= 0x00,
	.command				= 0b00000011, // fill, no chain
	.count					= (RRBSCREENWIDTH*50),
	.source					= (((FONTCHARMEM/64 + 0 /*star=10*/) >> 8)) & 0xff,
	.source_bank			= 0x00,
	.destination			= ((SCREEN + 1) & 0xffff),
	.destination_bank		= (((SCREEN + 1) >> 16) & 0x0f),
	.modulo					= 0x0000
};
