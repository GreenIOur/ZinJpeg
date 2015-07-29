
#include "zinjpeg_bitbuffer.hh"
#include "zinjpeg_printutils.hh"

#define BITBUFFER_DEBUG 0

static struct BitBuffer instance_;

int read_from_file(unsigned short* out);

void bitbuffer_construct(){
	instance_.buffer=0;
	instance_.buffer_size=0;
}

// Read a byte from file and adds it to the buffer.
static errorcode_t bitbuffer_buffer_refill() {
	unsigned short readed = 0;
	int count = read_from_file(&readed);

	if(count < 0)
		return count; // propagate negative errorcode

#if BITBUFFER_DEBUG
	printf("\n [BitBuf] Adding %d bits to buffer: readed = ", count);
	printbits(count, readed);
	printf("\n         old_size = %u", instance_.buffer_size);
#endif

	instance_.buffer = (instance_.buffer) << count;
	instance_.buffer = (instance_.buffer) | readed;
	instance_.buffer_size += count;

#if BITBUFFER_DEBUG
	printf("\n         new_size = %u \n", instance_.buffer_size);
#endif
	return 0; // Success
}

// Reads without consuming nr bits.
errorcode_t bitbuffer_peek_bits(unsigned int nr, unsigned int* out) {
	// Refill the buffer if necessary
	while(instance_.buffer_size < nr) {
	 	errorcode_t retval = bitbuffer_buffer_refill();
		if(retval < 0) {
			return retval;
		}
	}

	// Mask of size nr
	unsigned int mask = ((1<<nr) - 1);
	// Remove the not needed bits
	unsigned int readed = instance_.buffer >> (instance_.buffer_size - nr);
	// Make sure to bring only the nr bits
	*out = readed & mask ;



	// Success
	return 0;
}


// Discard bits from the buffer.
void bitbuffer_discard_bits(unsigned int nr) {
	// Reduce size
	instance_.buffer_size -= nr;
	// Check bounds
	assert(instance_.buffer_size <= 64);
	// Clear readed bits from buffer
	instance_.buffer = (instance_.buffer) & ((1<<instance_.buffer_size) - 1);
}

// Read bits from the buffer, consuming them.
errorcode_t bitbuffer_read_bits(unsigned int nr, unsigned int* out) {

	// Read bits
	errorcode_t retval = bitbuffer_peek_bits(nr, out);
	if(retval)
		return retval;

	bitbuffer_discard_bits(nr);

	return 0;
}


errorcode_t bitbuffer_starts_with(unsigned int nr, unsigned int bits) {
	if(nr == 0)
		return 0;

	unsigned int readed;
	unsigned int mask = (1<<(nr)) - 1;
	errorcode_t retval = bitbuffer_peek_bits(nr, &readed);
	if(retval < 0)
		return retval;

	readed = readed & mask;
	bits = bits & mask;
	return (readed == bits ? 1 : 0);
}


int bitbuffer_get_size() {
	return instance_.buffer_size;
}
