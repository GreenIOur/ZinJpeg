#ifndef _RADIO_STUB_
#define _RADIO_STUB_

#include <stdint.h>

void radio_init(char* buf_img, int dim);
void radio_sem();
int radio_read_packet(char **reader_ptr);

#endif
