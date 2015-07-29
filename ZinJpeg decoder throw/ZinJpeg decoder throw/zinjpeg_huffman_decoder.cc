#include "zinjpeg_huffman_decoder.hh"
#include "./jpeg/inc/zinjpeg_tables.h"
#include "zinjpeg_printutils.hh"
#include "zinjpeg_config.hh"
#include "zinjpeg_errors.hh"
#include <cstring>


// TODO: Find ERRPOINT and handle markers appropriately.

//////// Decoding Context

int last_dc = 0;


//////// Initialization: Sort the AC table in increasing order of lenght
uint8_t ac_table_order[256];
uint8_t ac_num_of_words;

static inline int index_to_zerorun(int index) {
	return index%16;
}

static inline int index_to_magn(int index) {
	return index/16;
}

/**
 * Sort the table in O(256*15). Optimally we can reach O(256*8)
 * with mergesort, but using more memory.
 */
void huff_table_init() {
	ac_num_of_words = 0;
	// From lenghts from 1 to 16.
	// Lenghts = 0 are invalid.
	for(int len = 1; len <= 16; len++) {
		// Scan the matrix
		for(int i = 0; i < 12*16; i++) {
			int zr = index_to_zerorun(i);
			int magn = index_to_magn(i);
			// If lenghts matches, then add it to array.
			if(HYAClen[zr][magn] == len) {
				// Add to array
				ac_table_order[ac_num_of_words] = i;

				// Advance of one position
				ac_num_of_words ++;
			}
		}
		printf("Sorting table: Pass %d/16 completed. %d items sorted.\n", len, (int)ac_num_of_words);
	}
}


//////// Utilities

int fix_negative_number(unsigned int data, int magn) {
	int output = data;
	if((data & (1<<(magn-1))) == 0){ // If the first bit is 0 then it is negative.
		// Correct the negative numbers:
		unsigned int mask = ((1<<magn)-1); // 00000111
		mask = ~mask;                      // 11111000
		output |=mask;                     // 00000010 | 11111000 = 11111010 add negative bits in front.
		output = output+1;                 // 11111010 + 1 = 11111011 transform into two's complement.
	}
	return output;
}

void huff_reset() {
	// Should reset everything
	last_dc = 0;
}
//////// Code

errorcode_t read_huff_dc_magn(int *output) {
	int match_result = 0;

	for(int i = 0; i < 12; i++) {

		// Try different values of dc, and check if they match.
		match_result = bitbuffer_starts_with(HYDClen[i], HYDCbits[i]);
		if(match_result == 1) {
			DEBUG_MATCHES("[HuffDC]\tMatched :", HYDClen[i], HYDCbits[i]);
			bitbuffer_discard_bits(HYDClen[i]);
			(*output) = i;
			return 0;
		} else if (match_result < 0) {
			return match_result;
		}
	}
	printf("[HuffDC]\tDC value not matched!\n");
	unsigned int tempdump;
	bitbuffer_read_bits(16, &tempdump);
	printf("Dump: ");
	printbits(16, tempdump);
	printf("\n");
	return ERR_UNMATCHED_SEQUENCE;
}

errorcode_t read_huff_dc( short *output) {
		int magn;
		errorcode_t retval;
		unsigned int data;
		retval = read_huff_dc_magn(&magn);
		if(retval < 0) {
			// ERRPOINT: Marker while reading magnitde!
			return retval;
		}

		// Continue reading
		retval = bitbuffer_read_bits(magn, &data);
		if(retval < 0) {
			// ERRPOINT: Marker when reading DC!
			return retval;
		}
		(*output) = fix_negative_number(data, magn);
		DEBUG_DC(magn, (int)data, *output);
		return 0;
}

// Note: we use field "value" to return the magnitude, not the coefficient!
errorcode_t read_huff_ac_magn(ac_coefficient_t *out) {
	errorcode_t retval = 0;
	// TODO: Sort for in "code length" order, or it will not handle markers appropriately.
	// Try different values of bitlen and zerorun, and check if they match.
	for(int index = 0; index < ac_num_of_words; index ++) {


		const int zr = index_to_zerorun(ac_table_order[index]);
		const int magn = index_to_magn(ac_table_order[index]);

		assert(HYAClen[zr][magn] != 0);

		// If it is a valid sequence.
		if(HYAClen[zr][magn] != 0) {
			retval = bitbuffer_starts_with(HYAClen[zr][magn], HYACbits[zr][magn]);
			if(retval == 1) {
				DEBUG_MATCHES("[HuffAC]\tMatched :", HYAClen[zr][magn], HYACbits[zr][magn]);

				bitbuffer_discard_bits(HYAClen[zr][magn]);
				out->zerorun = zr;
				out->value = magn;
				return 0;
			} else if(retval < 0) {
				// ERRPOINT: Marker when reading AC!
				// (or error if retval <= -256)
				return retval;
			}
		}
	}

	printf("[HuffAC]\tAC value not matched! \n");
	unsigned int tempdump;
	bitbuffer_read_bits(16, &tempdump);
	printf("Dump: ");
	printbits(16, tempdump);
	printf("\n");
	return ERR_UNMATCHED_SEQUENCE;
}

errorcode_t read_huff_ac(ac_coefficient_t* out) {
	// TODO Reusing the same struct is ugly.
	errorcode_t retval = read_huff_ac_magn(out);
	if(retval < 0)
		return retval;
	if(out->value != 0){
		// Normal coefficient
		unsigned int data;
		assert(out->value > 0);
		retval = bitbuffer_read_bits(out->value, &data);
		if(retval < 0)
			return retval;
		int output = fix_negative_number(data, out->value);
		DEBUG_AC(out->value, out->zerorun, output);
		out->value = output;
	}
	return 0;
}


// NOTE: Output is still in a zig-zagged order!
// You need to reorder it if you want to use it.
errorcode_t read_block(short* block) {
	memset(block, 0, NUM_COEFF * sizeof(short));

	// Read DC
	errorcode_t retval = read_huff_dc(&block[0]);
	if(retval < 0) {
		return retval;
	}

	block[0] += last_dc;
	last_dc = block[0];
	if(last_dc > 1024)
		last_dc = 1024;
	else if(last_dc < -1024)
		last_dc = -1024;
	block[0] = last_dc;

	// Read all the 63 AC coefficients
	for(int coeff = 1; coeff < 64; coeff++) {
		ac_coefficient_t readed;
		retval = read_huff_ac(&readed);
		if(retval < 0) {
			return retval;
		}

		// Handle End of Block
		if(readed.zerorun == 0 && readed.value == 0) {
			DEBUG_EOB(coeff);
			break; // We are done for this block.
		}

		// Set the value in the array, and then advance of zerorun (if zerorun != 0)
		// Note that we always advance by one in the for.
		// In this way the ZRL (16 zeroes) special case works without special code.
		coeff += readed.zerorun;
		if(coeff >= 64)
			return ERR_UNMATCHED_SEQUENCE;
		block[coeff] = readed.value;
	}

	return 0;
}
