#include <stdlib.h>

typedef unsigned char color;
/*
RGB to YCbCr Conversion:
*/
// Y = 0.299*R + 0.587*G + 0.114*B
inline color RGB2Y(const color r, const color g, const color b)
{
	return (153*r + 301*g + 58*b)>>9;
}
// Cb = -0.1687*R - 0.3313*G + 0.5*B + 128
inline color RGB2Cb(const color r, const color g, const color b)
{
	return (65536 - 86*r - 170*g + 256*b)>>9;
}
// Cr = 0.5*R - 0.4187*G - 0.0813*B + 128
inline color RGB2Cr(const color r, const color g, const color b)
{
	return (65536 + 256*r - 214*g - 42*b)>>9;
}

/*******************************
 * MANOET
 */
typedef struct bitbuffer_s
{
	unsigned buf;
	unsigned n;
}
bitbuffer_t;

#define ESCAPE 		0
#define NO_ESCAPE	1
void writebits(bitbuffer_t *const pbb, unsigned bits, unsigned nbits, short mod);
void writebyte(const unsigned char b);
void writeword(const unsigned short w);
void flushbits(bitbuffer_t *pbb);
void get_bitbuffer_info(bitbuffer_t *bb, short * n_ones, short *free_space);


/*
 * END OF MANOET
 *******************************/