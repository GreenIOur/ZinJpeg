
/* #include <stdlib.h> */
/* #include <iostream> */
/* #include <fstream> */
/* #include <string.h> */

#include "stats.h"
#include "jpeg.h"
#include "dct.h"
#include "zinzipeg_enc_interface.h"


unsigned short IMAGE_WIDTH  = 160;
unsigned short IMAGE_HEIGHT = 120;
#define BLOCK_SIZE          8
#define IMAGE_SIZE          (IMAGE_WIDTH * IMAGE_HEIGHT)

// Number of consecutive correlated block
// i.e. distance between consecutive markers
int CORRELATED_NUM = 5;
static unsigned short quality = 75;

void zinzipeg_set_quality(unsigned char q) {
	if(q > 100)
		q = 100;
	if(q < 1)
		q = 1;
	quality = q;
}

void zinzipeg_set_distance(unsigned char d) {
	CORRELATED_NUM = d;
}
void zinzipeg_set_size(unsigned short x, unsigned short y) {
	IMAGE_WIDTH = x;
	IMAGE_HEIGHT = y;
}


// Copy blocks of image into the buffer staring from top left of the image
static char get_block(unsigned x, unsigned y, unsigned sx, unsigned sy,
    unsigned char *image, unsigned char *block)
{
	if ((y + sy) > IMAGE_HEIGHT || (x + sx) > IMAGE_WIDTH) {
		return -1;
	}

	for (unsigned r = 0; r < sy; r++) {
		unsigned offset = IMAGE_WIDTH * (y+r) + x;

		for (unsigned c = 0; c < sx; c++) {
			unsigned i = offset + c;
			block[sy*r + c] = image[i];
		}
	}

	return 1;
}



int zinzipeg_encode(unsigned char* in_buf, unsigned char* out_buf) {

	unsigned char block[BLOCK_SIZE * BLOCK_SIZE];
	short block_o[BLOCK_SIZE * BLOCK_SIZE];

	init_buf(out_buf);

	huffman_start(IMAGE_HEIGHT, IMAGE_WIDTH, Y_IMAGE, quality);

	unsigned n = 0;
	for (unsigned y = 0; y < IMAGE_HEIGHT; y += 8) {
		for (unsigned x = 0; x < IMAGE_WIDTH; x += 8) {
			get_block(x, y, 8, 8, in_buf, block);
			dct3(block, block_o);
			block_o[0] -= 1024;
			huffman_encode(HUFFMAN_CTX_Y, block_o, n++);
		}
	}

	huffman_stop();

	return get_size();
}

unsigned zinzipeg_get_stats() {
	return GetNoOverheadStats();
}

unsigned zinzipeg_get_overhead_stats() {
	return GetOverheadStats();
}

void zinzipeg_reset_stats() {
	ResetStats();
}
