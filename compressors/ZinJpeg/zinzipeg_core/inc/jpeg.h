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

//---------------- J P E G ---------------
char init_jpeg(const char *name);
char init_buf(unsigned char* buffer);
// Application should provide this function for JPEG stream flushing
void write_jpeg(const unsigned char buff[], const unsigned size);
void close_jpeg(void);
int get_size();
int _count();

typedef struct huffman_s {
	const unsigned char  (*haclen)[12];
	const unsigned short (*hacbit)[12];
	const unsigned char  *hdclen;
	const unsigned short *hdcbit;
	const unsigned char  (*qtable)[64];
	short                dc;
} huffman_t;

extern huffman_t huffman_ctx[3];
unsigned char q_table[8][8];

#define	HUFFMAN_CTX_Y	&huffman_ctx[0]
#define	HUFFMAN_CTX_Cb	&huffman_ctx[1]
#define	HUFFMAN_CTX_Cr	&huffman_ctx[2]

void gen_table(unsigned q_fctr);
char huffman_start(short height, short width, image_type_t t, unsigned q_fctr);
void huffman_stop(void);
void huffman_encode(huffman_t *const ctx, const short data[64], unsigned block_num);


#ifdef __cplusplus
}
#endif

#endif//__JPEG_H__
