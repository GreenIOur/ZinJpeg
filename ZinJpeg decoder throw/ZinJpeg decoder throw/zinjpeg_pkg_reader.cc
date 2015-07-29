#include "zinjpeg_pkg_reader.hh"
#include <assert.h>
#include "zinjpeg_encoder.hh"
#include "zinjpeg_hamming.hh"
#include <string.h>
#include "zinjpeg_errors.hh"
#include "zinjpeg_config.hh"
#include <stdlib.h>
#include "zinjpeg_radio_stub.hh"

// Global instance of the pkg_reader
struct pkg_reader pkg_reader_;

// Instance of the current packet's trailer
extern trailer_s current_t;

/*******************************************************
	Interface functions
	A quick description for each interface function
	can be found in the header file pkg_reader.hh
*******************************************************/
void pkg_reader_construct() {
	pkg_reader_.bits_counter = 0;
	pkg_reader_.cur_t = init_trailer();
}


int pkg_reader_get(int* out) {
	uint16_t frame_bitsnumber = pkg_reader_.cur_t->numbits;
	// If the buffer has already been read we need to load a new frame
	if (pkg_reader_.bits_counter >= frame_bitsnumber) {
		int r = load_frame();
		// If we have already read all the frames
		if (r < 0) {
			*out = 0;
			return r;
		}
		// If we are here it means we are reading the continuation
		// of a block we already started to read
		// We need to update frame_bitsnumber value
		frame_bitsnumber = pkg_reader_.cur_t->numbits;
	}
	
	*out = (unsigned char) *(pkg_reader_.buffer + (pkg_reader_.bits_counter / 8));
	int read;
	if (pkg_reader_.bits_counter + 8 > frame_bitsnumber) {
		read = frame_bitsnumber - pkg_reader_.bits_counter;
		*out = *(out) >> (8 - read);
	}
	else read = 8;

	pkg_reader_.bits_counter += read;
	return read;
}


int get_first_block_index() {
	return pkg_reader_.cur_t->first_block;
}

int get_last_block_index() {
	return pkg_reader_.cur_t->last_block;
}

/*******************************************************
	Utility functions
*******************************************************/

/*
 * Calls the hamming decoder to obtain the data from the trailer and
 * stores them into the structure cur_t.
 */
static short get_trailer(const char* buffer) {
	uint64_t trailer;
	trailer = (((uint64_t) buffer[99]) & 0xff) << 32;
	trailer |= (((uint64_t) buffer[100]) & 0xff) << 24;
	trailer |= (((uint64_t) buffer[101]) & 0xff) << 16;
	trailer |= (((uint64_t) buffer[102]) & 0xff) << 8;
	trailer |= ((uint64_t) buffer[103]) & 0xff;

	trailer = SECDED_decoder(&trailer);

	/*
	 * flags (2 bits)
	 * last_block (13 bits)
	 * num_blocks (7 bits)
	 * numbits (10 bits)
	 */
	// Storing the former trailer and filling the new one
	swap_trailer();
	pkg_reader_.cur_t->flags = (uint8_t)(trailer >> 30);
	pkg_reader_.cur_t->last_block = (uint16_t)(trailer >> 17) & 0x1FFF;
	pkg_reader_.cur_t->num_blocks = (uint16_t)(trailer >> 10) & 0x7F;
	pkg_reader_.cur_t->numbits = (uint16_t)(trailer & 0x3FF);

	return get_safety_level();
}


/**
 * Loads a new frame. It uses trailer_checker functions
 * to ensure the correctness and trustness of the data
 * and perform some recovery operations.
 */
int load_frame() {
	char *ptr;
	
	printf("\n");
	DEBUG_PACKET("Loading a new packet");
	// If we already have data to read, as in the case of ACCEPT_BOTH
	radio_sem();
	if (radio_read_packet(&ptr) == -1) {
		// No more data to read from the radio
		DEBUG_PACKET("No more data to read");
		return ERR_EOF;
	}

	get_trailer(ptr);
	validate_trailer();

	memcpy(pkg_reader_.buffer, ptr, PCKT_DIM);

	DEBUG_PACKET("Accepted packet! Printing information:");
	DEBUG_PACKET_3(pkg_reader_.cur_t->flags, pkg_reader_.cur_t->last_block,
					pkg_reader_.cur_t->num_blocks, pkg_reader_.cur_t->numbits);
	pkg_reader_.bits_counter = 0;

	if (pkg_reader_.cur_t->flags >= F_CONT_FRAG)
		return 0;
	return ERR_PACKET_SWITCH;
}


/**
 * It locates the first marker in the frame and set it as the next byte to read.
 * This procedure is used whenever we think we received a packet which starts
 * with the continuation of one block whose beginning had been lost.
 * If no marker is found the load_frame is called and its returned value returned.
 */
int move_to_marker() {
	uint16_t frame_bitsnumber = pkg_reader_.cur_t->numbits;
	while (pkg_reader_.bits_counter < frame_bitsnumber) {
		const char *addr = pkg_reader_.buffer + pkg_reader_.bits_counter / 8;
		if ((*addr == (char)0xff) && (*(addr+1) != 0))
			return 0;
		pkg_reader_.bits_counter += 8;
	}
	return load_frame();
}
