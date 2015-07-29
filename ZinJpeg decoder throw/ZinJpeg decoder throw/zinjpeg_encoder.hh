#ifndef ENCODER_HH
#define ENCODER_HH

#include <fstream>

extern int written_blocks;

	void write_header(unsigned q);
	void write_block(short* coefficients);
	void write_trailer();
	void write_result(std::ofstream* file_out);
    unsigned char* write_result_to_buffer(int *size);


#endif
