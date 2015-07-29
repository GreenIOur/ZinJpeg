#include "zinjpeg_encoder.hh"
#include "jpeg/inc/zinjpeg_jpeg.h"
#include <cassert>
#include <iostream>
#include "zinjpeg_config.hh"


unsigned char out_buf[IMAGE_SIZE];
int written_blocks = 0;


void write_header(unsigned q_fctr) {
	init_buf(out_buf);
	huffman_start(IMAGE_HEIGHT, IMAGE_WIDTH, Y_IMAGE, q_fctr);
	written_blocks = 0;
}

void write_block(short* coefficients) {
	assert(written_blocks < NUM_OF_BLOCKS);
	/*if(written_blocks>= NUM_OF_BLOCKS) {
		assert(false);
		printf("[Encoder] WARNING: trying to write too many blocks.\n");
		return;
	}*/
	DEBUG_ENC(written_blocks);
	written_blocks++;
	huffman_encode(HUFFMAN_CTX_Y, coefficients);
}

void write_trailer() {
	huffman_stop();
}

void write_result(std::ofstream* file_out) {
	int size = get_size();
	printf("Writing %d bytes \n", size);
	file_out->write((char*)out_buf, size);
}

unsigned char* write_result_to_buffer(int* size){
    *size = get_size();
    printf("Writing %d bytes in buffer\n", *size);
    return out_buf;

}