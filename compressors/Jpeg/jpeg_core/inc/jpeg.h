#ifndef __JPEG_H__
#define __JPEG_H__

#ifdef __cplusplus
extern "C" {
#endif

#define JPEG_BUFFER     256

typedef enum {
	Y_IMAGE = 1,
	YCBCR_IMAGE,
	YIQ_IMAGE,
	CMYK_IMAGE,
} image_type_t;

typedef unsigned char color;
/*
RGB to YCbCr Conversion:
*/
// Y = 0.299*R + 0.587*G + 0.114*B
//inline color RGB2Y(const color r, const color g, const color b)
//{
//	return (153*r + 301*g + 58*b)>>9;
//}
//// Cb = -0.1687*R - 0.3313*G + 0.5*B + 128
//inline color RGB2Cb(const color r, const color g, const color b)
//{
//	return (65536 - 86*r - 170*g + 256*b)>>9;
//}
//// Cr = 0.5*R - 0.4187*G - 0.0813*B + 128
//inline color RGB2Cr(const color r, const color g, const color b)
//{
//	return (65536 + 256*r - 214*g - 42*b)>>9;
//}

//---------------- J P E G ---------------
char init_jpeg(const char *name);
char init_buf(unsigned char* buffer);
// Application should provide this function for JPEG stream flushing
void write_jpeg(const unsigned char buff[], const unsigned size);
void close_jpeg(void);
int get_size();
int _count();

unsigned char q_table[8][8];

typedef struct huffman_s
{
	const unsigned char  (*haclen)[12];
	const unsigned short (*hacbit)[12];
	const unsigned char  *hdclen;
	const unsigned short *hdcbit;
	const unsigned char  *qtable;
	short                dc;
}
huffman_t;

extern huffman_t huffman_ctx[3];

#define	HUFFMAN_CTX_Y	&huffman_ctx[0]
#define	HUFFMAN_CTX_Cb	&huffman_ctx[1]
#define	HUFFMAN_CTX_Cr	&huffman_ctx[2]

void gen_table(unsigned q_fctr);
char huffman_start(short height, short width, image_type_t t, unsigned q_fctr);
void huffman_stop(void);
void huffman_encode(huffman_t *const ctx, const short data[64]);


#ifdef __cplusplus
}
#endif

#endif//__JPEG_H__
