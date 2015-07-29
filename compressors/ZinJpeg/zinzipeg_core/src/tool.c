#include "tool.h"

// code-stream output counter
static unsigned jpgn = 0;
// code-stream output buffer, adjust its size if you need
static unsigned char jpgbuff[JPEG_BUFFER];

void writebyte(const unsigned char b)
{
	jpgbuff[jpgn++] = b;

	if (jpgn == sizeof(jpgbuff)) {
		jpgn = 0;
		write_jpeg(jpgbuff, sizeof(jpgbuff));
	}
}

void writeword(const unsigned short w)
{
	writebyte(w >> 8); writebyte(w);
}

/******************************************************************************
**  writebits
**  --------------------------------------------------------------------------
**  Write bits into bit-buffer.
**  If the number of bits exceeds 16 the result is unpredictable.
**  
**  ARGUMENTS:
**      pbb     - pointer to bit-buffer context;
**      bits    - bits to write;
**      nbits   - number of bits to write, 0-16;
**
**  RETURN: -
******************************************************************************/
void writebits(bitbuffer_t *const pbb, unsigned bits, unsigned nbits, short mod)
{
	// shift old bits to the left, add new to the right
   	pbb->buf = (pbb->buf << nbits) | (bits & ((1 << nbits)-1));

	nbits += pbb->n;
	
	// flush whole bytes
	while (nbits >= 8) {
		unsigned char b;

		nbits -= 8;
		b = pbb->buf >> nbits;

		writebyte(b);
		if (b == 0xFF && mod != NO_ESCAPE)// replace 0xFF with 0xFF00
			writebyte(0);
	}

	pbb->n = nbits;
}



/**
 * @param[out] n_ones number of consecutive ones which ends the buffer
 * @param[out] free_space number of free bits in the buffer
 */
void get_bitbuffer_info(bitbuffer_t *pbb, short * n_ones, short *free_space) {
	*free_space = 8 - pbb->n;
	*n_ones = 0;
	for (int i = 0; i < pbb->n; i++) {
		unsigned tmp = (pbb->buf >> i) & 0x01;
		if (tmp == 1) (*n_ones)++;
		else return;
	}
}
/******************************************************************************
**  flushbits
**  --------------------------------------------------------------------------
**  Flush bits into bit-buffer.
**  If there is not an integer number of bytes in bit-buffer - add zero bits
**  and write these bytes.
**  
**  ARGUMENTS:
**      pbb     - pointer to bit-buffer context;
**
**  RETURN: -
******************************************************************************/
void flushbits(bitbuffer_t *pbb)
{
	if(pbb->n != 0)
		writebits(pbb, 0, 8-pbb->n, NO_ESCAPE);
}