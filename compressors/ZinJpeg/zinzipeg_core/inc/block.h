#ifndef BLOCK_BUFFER_H
#define BLOCK_BUFFER_H

#include <stdint.h>
//#include "jpeg.h"


#define MAX_BLOCK_SIZE 		99
#define BLOCK_BUFFER_SIZE	150



typedef struct {
	uint8_t block[BLOCK_BUFFER_SIZE];
	unsigned size;
	unsigned block_num;
	uint8_t marker;
} block_buffer;

void block_start(unsigned block_num);
void block_end();
void add_to_block(unsigned bits, unsigned nbits);
void close_PDU(bitbuffer_t *bb);
short get_DC_value(short, unsigned);

void blocks_write_info();
void write_PDU_counter();

#endif
