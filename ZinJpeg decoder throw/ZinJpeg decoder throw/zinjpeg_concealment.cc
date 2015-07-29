#include "zinjpeg_config.hh"
#include "zinjpeg_encoder.hh"
#include "stdlib.h"
#include <cstring>
#include <cassert>
#define CONCEALMENT_FILE "last_concealed.bin"
#ifdef CONCEAL_GRAY

void concealment_initialize() {
	printf("CONCEALMENT: Gray\n");
}
void concealment_block_written_success(int /*blknr*/, short* /*block*/) {
}
void concealment_write_concealment_blocks(int /*blknr*/, int count) {
	short coefficients[NUM_COEFF];
	memset(coefficients, 0, NUM_COEFF * sizeof(short));
	for (int i = 0; i < count; i++)
		write_block(coefficients);

}
void concealment_finalize() {
}
#endif

/////////////////////////////////////////////////////////////
#ifdef CONCEAL_BLACK
void concealment_initialize() {
	printf("CONCEALMENT: Black\n");

}
void concealment_block_written_success(int /*blknr*/, short* /*block*/) {
}

void concealment_write_concealment_blocks(int /*blknr*/, int count) {
	short coefficients[NUM_COEFF];
	memset(coefficients, 0, NUM_COEFF * sizeof(short));
	coefficients[0] = -2048; // set block to black.
	for (int i = 0; i < count; i++)
		write_block(coefficients);
}
void concealment_finalize() {
}
#endif // CONCEAL_BLACK
/////////////////////////////////////////////////////////////
#ifdef CONCEAL_COPY
#include <fstream>

static short coeff[NUM_OF_BLOCKS*NUM_COEFF];

void concealment_initialize() {
	std::ifstream file_in(CONCEALMENT_FILE);
	printf("CONCEALMENT: Copy\n");
	if(!file_in) {
		//start with gray concealment
		memset(coeff, 0, NUM_COEFF * sizeof(short));
		printf("WARNING: Copy concealment not available: starting out with gray concealment\n");
		return;
	}
	file_in.read((char*)coeff, sizeof(short)*NUM_OF_BLOCKS*NUM_COEFF);
	assert(file_in.good());
	file_in.close();
}

void concealment_block_written_success(int blknr, short* block) {
	// copy it into the concealment buffer
	memcpy(&coeff[blknr*NUM_COEFF], block, sizeof(short)*NUM_COEFF);
}
void concealment_write_concealment_blocks(int blknr, int count) {
	for(int idx = blknr; idx < blknr + count; idx++) {
		write_block(&coeff[idx*NUM_COEFF]);
	}
}
void concealment_finalize() {
	// save temp buffer to file, deallocate everything
	std::ofstream file_out(CONCEALMENT_FILE);
	file_out.write((char*)coeff, sizeof(short)*NUM_OF_BLOCKS*NUM_COEFF);
	assert(file_out.good());
	file_out.close();
}
#endif
