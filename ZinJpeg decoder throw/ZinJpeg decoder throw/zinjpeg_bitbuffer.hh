#include <iostream>
#include <fstream>
#include <cassert>
#include <stdint.h>
#include "zinjpeg_printutils.hh"

// The bitbuffer object. Don't touch internal fields.
struct BitBuffer {
	uint64_t buffer;
	unsigned int buffer_size;
};

//! Constructs the bit buffer.
void bitbuffer_construct();

//! Reads bits from the bitbuffer, without consuming them.
errorcode_t bitbuffer_peek_bits(unsigned int nr, unsigned int* out);

//! Reads bits from the bitbuffer, consuming them.
errorcode_t bitbuffer_read_bits(unsigned int nr, unsigned int* out);

//! Discards bits from the bitbuffer.
void bitbuffer_discard_bits(unsigned int nr);

//! Checks if the buffer starts with the specified.
errorcode_t bitbuffer_starts_with(unsigned int nr, unsigned int bits);

//! Gets the number of bits inside the bitbuffer
int bitbuffer_get_size();
