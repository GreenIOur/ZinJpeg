#include <stdio.h>
#include <cassert>
#include "zinjpeg_trailer_checker.hh"
#include "zinjpeg_config.hh"


// Global instance of the current and last trailer
trailer_s current_t;
trailer_s old_trailer;

static void boundaries_check(void);
static void compute_first_block(void);


/**************************************
	Interface functions
**************************************/
void swap_trailer(void) {

	current_t.flags ^= old_trailer.flags;
	old_trailer.flags ^= current_t.flags;
	current_t.flags ^= old_trailer.flags;

	current_t.last_block ^= old_trailer.last_block;
	old_trailer.last_block ^= current_t.last_block;
	current_t.last_block ^= old_trailer.last_block;

	current_t.num_blocks ^= old_trailer.num_blocks;
	old_trailer.num_blocks ^= current_t.num_blocks;
	current_t.num_blocks ^= old_trailer.num_blocks;

	current_t.numbits ^= old_trailer.numbits;
	old_trailer.numbits ^= current_t.numbits;
	current_t.numbits ^= old_trailer.numbits;

	current_t.first_block ^= old_trailer.first_block;
	old_trailer.first_block ^= current_t.first_block;
	current_t.first_block ^= old_trailer.first_block;
}

trailer_s* init_trailer() {
	current_t.flags = 0;
	// Last block must be initialized to -1. This is needed because
	// we start counting the block from 0
	old_trailer.last_block = current_t.last_block = -1;	
	old_trailer.num_blocks = current_t.num_blocks = 0;
	old_trailer.numbits = current_t.numbits = 0;
	return &current_t;
}

enum FsmAlignLocation{
	// for position fsm
	FSM_ALIGNED,
	FSM_MOVED,
};
enum FsmFragLocation{
	// for fragmentation fsm
	FSM_NORMAL,
	FSM_FRAGMENT
};

typedef unsigned char boolean;

void validate_trailer() {

	// Easy checks to ensure all the boundaries are respected.
	boundaries_check();

	compute_first_block();


    // For the next section I really really hope that the upper layer
    // function calls this function only once and properly sets the
    // current_t and old_trailer variables. Considering the number of
    // times the swap_trailer function is called this is almost
    // impossible, but, you know, hope is the last thing to die...
    //
    // If you are trying to make sense of the flow of control in this
    // project, I feel sorry for you.

    // FSM as in Finite State Machine
	// FSM: Position

	// global variables holding fsm state
	static enum FsmAlignLocation fsm_loc_position = FSM_ALIGNED;
	static int fsm_last_original_position = -1;

	// actions to perform
	boolean modify_last_block = 0; // false
	boolean modify_numblocks = 0; // false

	// conditions
	const boolean is_aligned_orig = (current_t.first_block - 1 == fsm_last_original_position);
	const boolean is_aligned_prev = (current_t.first_block - 1 == old_trailer.last_block);
	const boolean is_before_prev  = (current_t.first_block - 1 <  old_trailer.last_block);

	// fsm
	switch(fsm_loc_position) {
	case FSM_ALIGNED:
		if (is_aligned_prev)     { /* accept, remain in aligned */}
		else if (is_before_prev) { modify_last_block = 1; fsm_loc_position = FSM_MOVED; }
		else  /* is after */     { modify_numblocks = 1; fsm_loc_position = FSM_MOVED;  }
		break;
	case FSM_MOVED:
		if(is_aligned_prev)     { fsm_loc_position = FSM_ALIGNED;}
		else if(is_aligned_orig){ fsm_loc_position = FSM_ALIGNED;}
		else if(is_before_prev)  { modify_last_block = 1; fsm_loc_position = FSM_MOVED; }
		else /* is after */     { modify_numblocks = 1; fsm_loc_position = FSM_MOVED; }
		break;
	}
	fsm_last_original_position = current_t.last_block;

	// perform actions
	if(modify_last_block) {
		current_t.last_block = old_trailer.last_block + current_t.num_blocks;
		boundaries_check();
		compute_first_block();
	} else if(modify_numblocks) {
		current_t.num_blocks = current_t.last_block - old_trailer.last_block;
		boundaries_check();
		compute_first_block();
	}

	// FSM 2: Fragmentation

	// global variables holding fsm state
	static enum FsmFragLocation fsm_loc_frag = FSM_NORMAL;

	// actions to perform
	boolean action_fragment = 0; // false

	// conditions
	const boolean is_continue  = (current_t.flags == F_CONT_FRAG);
	const boolean is_end  = (current_t.flags == F_END_FRAG);
	const boolean has_one_block = (current_t.num_blocks == 1);
	const boolean has_zero_block = (current_t.num_blocks == 0);
	const boolean was_moved = modify_last_block || modify_numblocks ;

	// fsm
	switch(fsm_loc_frag) {
	case FSM_NORMAL:
		if(has_one_block)        { action_fragment=0; fsm_loc_frag = FSM_FRAGMENT; }
		else                     { action_fragment=0; fsm_loc_frag = FSM_NORMAL; }
		break;
	case FSM_FRAGMENT:
		if(is_continue && has_zero_block && !was_moved) { action_fragment=1; fsm_loc_frag = FSM_FRAGMENT; }
		else if(is_end && !was_moved)                   { action_fragment=1; fsm_loc_frag = FSM_NORMAL; }
		else                                            { action_fragment=0; fsm_loc_frag = FSM_NORMAL; }
		break;
	}

	// perform actions
    if(action_fragment) {
        current_t.flags = F_CONT_FRAG;
    } else {
        current_t.flags = F_NO_FRAG;
    }
}


/**************************************
	Other functions
**************************************/

//! It computes the value of the first block field.
static void compute_first_block(void) {
	if (current_t.num_blocks > 0) {
		if((current_t.num_blocks - 1) <= current_t.last_block)
			// the values are sane
			current_t.first_block = current_t.last_block - (current_t.num_blocks - 1);
		else
			// avoid putting negative values in unsigned variables
			current_t.first_block = 0;
	} else
		current_t.first_block = current_t.last_block;
}

/**
 * This function performs some simple check on the trailer values.
 */
static void boundaries_check(void) {
	if (current_t.numbits > MAX_PDU_LENGTH) {
		current_t.numbits = MAX_PDU_LENGTH;
	}
	if (current_t.num_blocks > NUM_OF_BLOCKS) {
		current_t.num_blocks = NUM_OF_BLOCKS;
	}
	if (current_t.last_block > NUM_OF_BLOCKS) {
		current_t.last_block = NUM_OF_BLOCKS;
	}
}
