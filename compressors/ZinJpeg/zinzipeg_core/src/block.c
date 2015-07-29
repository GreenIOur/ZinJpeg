#include "block.h"
#include "jpeg.h"
#include "hamming.h"
#include "stats.h"
#include <string.h>
#include <assert.h>
#include "tool.h"
// Maximum payload of a ZigBee packet is 104 byte, but we need a 5 byte trailer
#define PACKET_SIZE		104
#define TRAILER_SIZE	5
#define MAX_PDU_LENGTH 	(PACKET_SIZE - TRAILER_SIZE) * 8

// Number of consecutive correlated block
// i.e. distance between consecutive markers
extern int CORRELATED_NUM;

//! Number of bits which are contained in the current PDU
static uint16_t PDUbits = 0;

//! Number of the last block inserted in the current PDU
static uint16_t last_block = 0;

//! Number of blocks contained in the current PDU
static uint16_t block_counter = 0;

//! Buffer in which is stored the current block
block_buffer current_block;

uint8_t force_marker = 0;

//! PDU flags
uint8_t PDUfragment = 0;
uint8_t PDUfragment_end = 0;
uint8_t PDUfragment_start = 0;

/*******************************************************************
 *	Functions declarations
 ******************************************************************/
static void add_to_PDU(bitbuffer_t *bb);
static int check_stuffing(bitbuffer_t *bb);
static int compute_padding(void);
static uint8_t get_byte(int index, int *n);
static unsigned add_frag_to_PDU(bitbuffer_t *bb, unsigned offset);

/*******************************************************************
 *	Functions which manage blocks
 ******************************************************************/
/**
 * This function must be called whenever we are starting  a new block.
 * It cleans and sets some variable. If needed, it adds the marker at the begin of the block
 * @param[in] block_num number of the block which is starting to be analyzed
 */
void block_start(unsigned block_num) {
	// Cleaning the block buffer. Mandatory since we perform some OR operations on it
	memset(current_block.block, 0, 128);
	current_block.block_num = block_num;
	current_block.size = 0;

	// Writing the marker if needed.
	// We have to force it if the previous block was fragmented
	if (((block_num != 0) && (block_num % CORRELATED_NUM == 0)) || (force_marker == 1) ) {
		current_block.block[0] = 0xFF;
		current_block.size = 8;
		current_block.marker = 1;
		force_marker = 0;
	}
	else
		current_block.marker = 0;
}

/**
 * Add some bits to the current block
 * @param[in] bits the bits to be added
 * @param[in] nbits number of bits to be added
 */
void add_to_block(unsigned bits, unsigned nbits) {
	// Cleaning the variable from spurious values
	bits = (bits & ((1 << nbits)-1));
	unsigned actual_size = current_block.size;
	current_block.size += nbits;
	while (nbits > 0) {
		uint8_t tmp = 0;
		unsigned index = actual_size / 8;
		unsigned free_space = 8 - (actual_size % 8);
		if (nbits > free_space) {
			tmp = (uint8_t) (bits >> (nbits - free_space));
			current_block.block[index] |= tmp;
			actual_size += free_space;
			nbits -= free_space;
		}
		else {
			tmp = (uint8_t) (bits << (free_space - nbits));
			current_block.block[index] |= tmp;
			actual_size += nbits;
			nbits = 0;
		}
	}
	assert(current_block.size <= BLOCK_BUFFER_SIZE * 8);
}

/**
 * Close a block. If possible the block is added to the current PDU
 * If the block size is too big, the current PDU is closed and the block is inserted in
 * a new one.
 * The number of bits lost due to marker alignment and marker escaping
 * must be taken in account.
 */
void block_end(bitbuffer_t *bb) {
	// Check the number of additional bits will be needed to escape
	// bytes set to 0xFF
	int stuff_and_pad = check_stuffing(bb);
/*	int tmp = stuff_and_pad;*/

	// Markers must be at the beginning of a byte
	// if current block contains a marker we must consider
	// an additional padding needed to align the marker
	stuff_and_pad += compute_padding();

	// If the block is too large to be contained in the current PDU, close the PDU
	// This operation automatically "open" a new PDU
	if (PDUbits + current_block.size + stuff_and_pad > MAX_PDU_LENGTH) {
		close_PDU(bb);
		// If we have opened a new PDU we must re-check for stuffing
		// No check for padding is needed since the marker will be automatically
		// placed in the first byte of the new PDU
		stuff_and_pad = check_stuffing(bb);
/*		tmp = stuff_and_pad;*/
	}

	// Check whether the block is too big to fit into a packet
	if (current_block.size + stuff_and_pad > MAX_PDU_LENGTH) {
		last_block = current_block.block_num;
		block_counter++;
		unsigned written = 0;
		PDUfragment_start = 1;
		while (written < current_block.size) {
			written += add_frag_to_PDU(bb, written);
			if (written < current_block.size) {
				PDUfragment = 1;
				close_PDU(bb);
			}
		}
		PDUfragment_end = 1;
		force_marker = 1;
		return;
	}

	// Insert the block into the PDU
	add_to_PDU(bb);
	PDUbits += stuff_and_pad;
	assert(PDUbits <= MAX_PDU_LENGTH);
	block_counter++;
	last_block = current_block.block_num;
	// We can not write more than 127 blocks in a packet due to the limited
	// dimension of the block_num trailer field
	if (block_counter == 127) close_PDU(bb);
}

/*******************************************************************
 *	Functions which manage PDU
 ******************************************************************/

//! Add the current block to the current PDU 
static void add_to_PDU(bitbuffer_t *bb) {
	PDUbits += current_block.size;
	unsigned to_read = current_block.size;
	int i = 0;
	while (to_read > 0) {
		unsigned n = 0;
		unsigned tmp = get_byte(i++, (int*) &n);
		// If there is a marker we need to align it to a byte
		// and we have to not escape the first byte of the block
		if(i == 1 && current_block.marker == 1) {
			flushbits(bb);
			writebits(bb, tmp, n, NO_ESCAPE);
		}
		else
			writebits(bb, tmp, n, ESCAPE);
		to_read -= n;
	}
}


/**
 * Add a fragmented block to the PDU
 * NOTE: for performance reasons it does not go through
 * 		the writebits function. Escaping is performed
 *		manually, instead.
 */
static unsigned add_frag_to_PDU(bitbuffer_t *bb, unsigned offset) {
	unsigned written = 0;
	while (written + offset < current_block.size) {
		unsigned n = 0;
		unsigned index = (offset + written) / 8;
		unsigned char tmp = get_byte(index, (int*) &n);
		if (n < 8) {
			// Because the block is byte-aligned n<8 means this is the end of the block.
			// We don't have to worry about escape: a partial byte can never be escaped.
			writebits(bb, tmp, n, NO_ESCAPE);
			written += n;
			PDUbits += n;
			break;
		} else {
			writebyte(tmp);
			PDUbits += 8;
			written += 8;
			if (tmp == 0xff && (written + offset != 8|| current_block.marker != 1)) {
				writebyte(0);
				PDUbits += 8;
			}
		}
		// End if there is not remaining space in the PDU
		// The minus 8 is needed since the following byte may require escaping.
		if (PDUbits > MAX_PDU_LENGTH - 8) break;
	}
	return written;
}
/*	while (PDUbits < MAX_PDU_LENGTH - 8) {
		unsigned n = 0;
		unsigned index = offset + writtenB++;
		if (index > current_block.size / 8) break;
		unsigned char tmp = get_byte(index, &n);
		if (n == 8) {
			writebyte(tmp);
			PDUbits += 8;
			if (tmp == 0xff && (writtenB + offset != 1 || current_block.marker != 1)) {
				writebyte(0);
				PDUbits += 8;
			}
		} else {
			// Note that a partial byte will never need to be escaped,
			// hence we can be sure on the increment of PDUbits
			writebits(bb, tmp, n, ESCAPE);
			PDUbits += n;
		}
	}
	return writtenB * 8;
}*/


/**
 * Flush the current PDU with bit set to 0
 */
static void flush_PDU(bitbuffer_t *bb) {
	flushbits(bb);
	while (MAX_PDU_LENGTH - PDUbits >= 16) {
		writeword(0x0000);
		PDUbits += 16;
	}
	while (MAX_PDU_LENGTH - PDUbits >= 8) {
		writebyte(0x00);
		PDUbits += 8;
	}
}

/**
 * Two flags are currently used in the trailer. The meaning is the following
 * 00 - no fragmented block is present
 * 01 - the PDU contains the beginning of a fragmented block
 * 10 - the PDU contains the continuation of a fragmented block, but not the end of it
 * 11 - the PDU contains the end of a fragmented block
 */
static uint8_t compute_flags() {
	assert(!(PDUfragment_end == 1 && PDUfragment_start == 1));
	if (PDUfragment + PDUfragment_start + PDUfragment_end == 0)
		return 0;
	if (PDUfragment_start == 1)
		return 1;
	if (PDUfragment_end == 1)
		return 3;
	return 2;
}

/**
 * It closes the current PDU.
 * There is no need to open a new one
 */
void close_PDU(bitbuffer_t *bb) {
	// CAREFUL!!! PDUbits is modified by flush_PDU.
	// Be very careful if you choose to modify the order
	// of the instructions!

	// Computing the trailer for this PDU
	uint32_t trailer_clear;		// it contains the information that will be coded by hamming
	uint64_t trailer_enc;		// the hamming coded trailer
	trailer_clear = compute_flags();
	trailer_clear = trailer_clear << 30;
	trailer_clear |= (last_block << 17) | (block_counter << 10) | PDUbits;
	
	SECDED_encoder(trailer_clear, &trailer_enc);

	OverheadData(PACKET_SIZE);
	NoOverheadData(PDUbits);
	
	// Flushing and writing
	flush_PDU(bb);
	writebyte(trailer_enc >> 32);
	writeword((trailer_enc >> 16) & 0xFFFF);
	writeword(trailer_enc & 0x0000FFFF);
	
	PDUbits = 0;
	PDUfragment = 0;
	PDUfragment_end = 0;
	PDUfragment_start = 0;
	block_counter = 0;
}


/*******************************************************************
 *	Regarding padding and escaping
 ******************************************************************/
/**
 * Returns the number of bits needed to align the marker
 * to a byte
 */
static int compute_padding(void) {
	// If no marker present, no padding needed
	if (current_block.marker == 0)
		return 0;
	
	int remainder = PDUbits % 8;
	return (remainder == 0) ? 0 : (8 - remainder);
}



/**
 * Check whether the first number bits of current_block.block[index] are set to 1 or not.
 */ 
static short check_SOByte(short number, short index) {
	uint8_t byte = 0xff >> number;
	byte |= current_block.block[index];
	if (byte == 0xff)
		return 1;
	return 0;
}

/**
 * Check whether the last number bits of current_block.block[index] are set to 1 or not.
 */
static short check_EOByte(short number, short index) {
	uint8_t byte = 0xff << number;
	byte |= current_block.block[index];
	if (byte == 0xff)
		return 1;
	return 0;
}

/**
 * This function is used to perform a less complex test in the case in which
 * the beginning of the block is byte-aligned
 */
static int check_stuffing_with_marker(void) {
	int stuffing = 0;
	// If the marker is present we have to start analyzing from 1
	// because the marker of course should not be escaped.
	// If no marker is present, instead, we have to analyze from the beginning
	int i = current_block.marker;
	int last_to_check = current_block.size / 8;
	for (; i < last_to_check; i++)
		if (current_block.block[i] == 0xff)
			stuffing += 8;
	return stuffing;
}

/**
 * Returns the number of bits will be needed to perform the byte stuffing
 * we have to take in account the marker
 */
static int check_stuffing(bitbuffer_t *bb) {
	// If there is a marker we can perform a simpler test
	if (current_block.marker == 1)
		return check_stuffing_with_marker();

	// Retrieving the future alignment of this block
	// and the number of conseguent 1 which end the previous block
	short n_ones, free_space;
	int stuffing = 0;
	get_bitbuffer_info(bb, &n_ones, &free_space);

	// Again, if the beginning of the block is byte-aligned
	// we can perform a simpler test
	if (free_space == 8)
		return check_stuffing_with_marker();

	// This is executed when no marker is present
	// and the block is not byte-aligned

	// Doing a check which consider the end of the previous block
	// and the beginning of the new one
	if (free_space + n_ones == 8 && check_SOByte(8 - n_ones, 0))
		stuffing += 8;
	
	//check for all the bytes
	int last_byte_to_check = current_block.size / 8;
	unsigned int i;
	for(i = 0; i < last_byte_to_check; i++) {
		if (check_EOByte(8-free_space, i) == 1 && check_SOByte(free_space, i+1) == 1)
			stuffing += 8;
	}
	return stuffing;
}


/*******************************************************************
 *	Misc
 ******************************************************************/	
static uint8_t get_byte(int index, int *n) {
	unsigned last_byte_slack = current_block.size % 8;
	// If I can read a whole byte
	if (( last_byte_slack == 0) || (index < current_block.size / 8)) {
		(*n) = 8;
		return current_block.block[index];
	}
	// If I have to read less than a byte
	uint8_t tmp = current_block.block[index] >> (8 - last_byte_slack);
	tmp = (tmp & ((1 << last_byte_slack)-1));
	(*n) = last_byte_slack;
	return tmp;
}


short prev_dc;


/**
 * WARNING: in order to deal with megablocks, this function must be called
 * 			before block_start function.
 *			This is needed because block_start clears force_marker value.
 */
short get_DC_value(short dc, unsigned block_num) {
	short tmp = dc - prev_dc;
	prev_dc = dc;
	if (block_num % CORRELATED_NUM == 0 || force_marker == 1)
		return dc;
	return tmp;
}
