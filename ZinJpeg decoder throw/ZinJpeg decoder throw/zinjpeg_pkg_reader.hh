#ifndef _PKG_READER_
#define _PKG_READER_

#include <fstream>
#include <stdint.h>
#include "zinjpeg_trailer_checker.hh"
#include "zinjpeg_config.hh"


#define PR_BUFFER_SIZE 	PCKT_DIM /* *2 */

/**
 * Reads single frames containing some jpeg blocks and
 * provides some functions to get byte from them
 */
struct pkg_reader {
	char buffer[PR_BUFFER_SIZE];
	trailer_s *cur_t;			//! Trailer of the current frame
	uint16_t bits_counter;		//! Number of the bits read from the current frame
};

//! Constructs a new pkg_reader.
void pkg_reader_construct();

int load_frame();

/**
 * Gets some bits from the current frame
 * @param[out] out the read value if the returned value is positive, 0 if returns ERR_EOF, the number of missing packets if returns ERR_MISSING_PACKET
 * @return the number of bits read or a negative error code
 */
int pkg_reader_get(int* out);

//! Returns the number of the first block of the current frame.
int get_first_block_index();

//! Returns the number of the last block of the current frame.
int get_last_block_index();

//! Move to the next marker in the buffer
int move_to_marker();

#endif
