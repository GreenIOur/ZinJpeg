#include "zinjpeg_radio_stub.hh"
#include "zinjpeg_config.hh"

char* buffer;
int buffer_size;
int offset;
extern int packet_num;

void radio_init(char* buf_img, int dim) {
	buffer = buf_img;
	offset = -PCKT_DIM;
	buffer_size = dim;
}

/**
 * Unimplemented. This is only needed in order to simulate the
 * operations that will be performed on the board.
 */
void radio_sem() {}

int radio_read_packet(char **reader_ptr) {
	packet_num++;
	offset += PCKT_DIM;
	
	/* Simple check to avoid buffer overflows
	 * NOTE Keep >=! If you put here ==, SIG_SEGV errors may appear!
	 * This is due to the fact that this function might be called more times
	 * than necessary due to errors in the read bits.
	 */
	if (offset >= buffer_size)
		return -1;

	*reader_ptr = buffer + offset;
	return 0;	
}
