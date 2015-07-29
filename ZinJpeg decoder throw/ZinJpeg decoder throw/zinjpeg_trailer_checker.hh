#ifndef _CHECKER_
#define _CHECKER_

#include <stdint.h>


// Flags definition
#define F_NO_FRAG		0
#define F_INIT_FRAG		1
#define F_CONT_FRAG		2
#define F_END_FRAG		3

//! Stores the information contained in and about a given trailer
typedef struct {
	uint8_t flags;			//! Fragmentation flags
	uint16_t last_block;	//! Last block field of the trailer
	uint16_t num_blocks;	//! Number of blocks contained in the frame
	uint16_t numbits;		//! Number of useful bits in the frame
	uint16_t first_block;
} trailer_s;

//! Initializes the two main structures used by this module
trailer_s* init_trailer();

//! Analyzes a trailer, tries some recovery and returns a response
/**
 * Check the correctness of the data contained in a trailer
 * and decide which action has to be performed.
 */
void validate_trailer();

//! Swap the contents of old_trailer and cur_t
void swap_trailer(void);


#endif
