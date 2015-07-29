#ifndef HUFFMAN_DECODER_HH
#define HUFFMAN_DECODER_HH

#include "zinjpeg_bitbuffer.hh"

// [[ Public ]]

// Reads a coefficient block (DC + AC)
// Returned array is of 64 elements
//
// NOTE: Output is still in a zig-zagged order.
// You need to reorder it if you want to use it.
errorcode_t read_block(short* coefficients);


typedef struct ac_coefficient {
	int zerorun;
	int value;
} ac_coefficient_t;


//! Sorts the huffman table by length
void huff_table_init();
//! Reset the last_dc value to 0
void huff_reset();

// [[ Private ]]

// Fixes the representation of negative numbers.
// From the jpeg standard to c standard representation.
// If the number is positive then it will directly return it.
// data: the number to convert
// magn: the size in bits of the number
// returns: the fixed number
int fix_negative_number(unsigned int data, int magn);

// Reads the magnitude of a dc (decoding huffman)
errorcode_t read_huff_dc_magn(int *output);

// Reads a dc value and returns it.
errorcode_t read_huff_dc(short *output);

// Reads the magnitude of an AC component.
// Also reads the zerorun.
//
// Note: uses field "value" to return the magnitude.
errorcode_t read_huff_ac_magn(ac_coefficient_t* out);

// Reads an AC coefficient and the zerorun.
errorcode_t read_huff_ac(ac_coefficient_t* out);

#endif
