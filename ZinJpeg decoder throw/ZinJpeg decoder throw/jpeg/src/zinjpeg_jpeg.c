////////////////////
//
// Modified in order to remove zigzag table and quantization of coefficients.
// Inputs should already be quantized and reordered with the zigzag.
// Also, we avoid caching the "1024/x" for the quality tables, since
//  they are used only for the header.
// Copied from revision b9c7e08ca9ccd7d5713355e37ffdb3190fb83da6
//
////////////////////

#include "zinjpeg_jpeg.h"
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "zinjpeg_tables.h"
#include <string.h>
#include <math.h>

FILE *fp;

huffman_t huffman_ctx[3] =
{
	
	{HYAClen, HYACbits, HYDClen, HYDCbits, qtable_lum,   0}, // Y
	{HCAClen, HCACbits, HCDClen, HCDCbits, qtable_chrom, 0}, // Cb
	{HCAClen, HCACbits, HCDClen, HCDCbits, qtable_chrom, 0}, // Cr
};


typedef struct bitbuffer_s
{
	unsigned buf;
	unsigned n;
}
bitbuffer_t;

static bitbuffer_t bitbuf;

/******************************************************************************/

static int file_jpg = 0;

static int count = 0;
static unsigned char *b;

char init_jpeg(const char *name)
{
	if ((file_jpg = open(name, O_CREAT|O_TRUNC|O_RDWR, ~0)) < 0) 
		return -1;
	return 0;
}

char init_buf(unsigned char* buffer)
{
	b = buffer;
	return 1;
}

void write_jpeg(const unsigned char *buff, const unsigned size)
{
	memcpy(b + count, buff, size);
	count += size;
}

int _count()
{
	int im_size = count;
	return im_size;
}

int get_size()
{
	int im_size = count;
	count = 0;
	// TODO Simone: azzeramento qua non va bene
	return im_size;
}

void close_jpeg(void)
{
	close(file_jpg);
}
/******************************************************************************/


// code-stream output counter
static unsigned jpgn = 0;
// code-stream output buffer, adjust its size if you need
static unsigned char jpgbuff[JPEG_BUFFER];

static void writebyte(const unsigned char b)
{
	jpgbuff[jpgn++] = b;

	if (jpgn == sizeof(jpgbuff)) {
		jpgn = 0;
		write_jpeg(jpgbuff, sizeof(jpgbuff));
	}
}

static void writeword(const unsigned short w)
{
	writebyte(w >> 8); writebyte(w);
}

static void write_APP0info(void)
{
	writeword(0xFFE0); //marker
	writeword(16);     //length
	writebyte('J');
	writebyte('F');
	writebyte('I');
	writebyte('F');
	writebyte(0);
	writebyte(1);//versionhi
	writebyte(1);//versionlo
	writebyte(0);//xyunits
	writeword(1);//xdensity
	writeword(1);//ydensity
	writebyte(0);//thumbnwidth
	writebyte(0);//thumbnheight
}

// should set width and height and image type before writing
static char write_SOF0info(const short height, const short width, image_type_t t)
{
	switch(t) {
	case Y_IMAGE:
        	writeword(0xFFC0);	//marker
		writeword(11);		//length
		writebyte(8);		//precision
		writeword(height);	//height
		writeword(width);	//width
		writebyte(1);		//nrofcomponents
		writebyte(1);		//IdY
		writebyte(0x11);	//HVY, 4:2:0 subsampling
		writebyte(0);		//QTY
		return 1;
        	break;

	case YCBCR_IMAGE:
        	writeword(0xFFC0);	//marker
		writeword(17);		//length
		writebyte(8);		//precision
		writeword(height);	//height
		writeword(width);	//width
		writebyte(3);		//nrofcomponents
		writebyte(1);		//IdY
		writebyte(0x22);	//HVY, 4:2:0 subsampling
		writebyte(0);		//QTY
		writebyte(2);		//IdCb
		writebyte(0x11);	//HVCb
		writebyte(1);		//QTCb
		writebyte(3);		//IdCr
		writebyte(0x11);	//HVCr
		writebyte(1);		//QTCr
		return 1;
		break;
	case YIQ_IMAGE:
	case CMYK_IMAGE:
	default:
		return -1;
		break;
	}
	return 1;
}

static char write_SOSinfo(image_type_t t)
{
	switch(t) {
	case Y_IMAGE:
		writeword(0xFFDA);	//marker
		writeword(8);		//length
		writebyte(1);		//nrofcomponents
		writebyte(1);		//IdY
		writebyte(0);		//HTY
		writebyte(0);		//Ss
		writebyte(0x3F);	//Se
		writebyte(0);		//Bf 
		return 1;
		break;

	case YCBCR_IMAGE:
		writeword(0xFFDA);	//marker
		writeword(12);		//length
		writebyte(3);		//nrofcomponents
		writebyte(1);		//IdY
		writebyte(0);		//HTY
		writebyte(2);		//IdCb
		writebyte(0x11);	//HTCb
		writebyte(3);		//IdCr
		writebyte(0x11);	//HTCr
		writebyte(0);		//Ss
		writebyte(0x3F);	//Se
		writebyte(0);		//Bf
		return 1;
		break;
	case YIQ_IMAGE:
	case CMYK_IMAGE:
	default:
		return -1;
		break;
	}
	
}

static void write_DQTinfo(void)
{
	unsigned i;
	
	writeword(0xFFDB);
	writeword(132);
	writebyte(0);

	for (i = 0; i < 64; i++) 
		writebyte(((unsigned char*)huffman_ctx[0].qtable)[zig[i]]); // zig-zag order

	writebyte(1);

	for (i = 0; i < 64; i++) 
		writebyte(((unsigned char*)qtable_0_chrom)[zig[i]]); // zig-zag order
}

static void write_DHTinfo(void)
{
	unsigned i;
	
	writeword(0xFFC4); // marker
	writeword(0x01A2); // length

	writebyte(0); // HTYDCinfo
	for (i = 0; i < 16; i++) 
		writebyte(std_dc_luminance_nrcodes[i+1]);
	for (i = 0; i < 12; i++) 
		writebyte(std_dc_luminance_values[i]);

	writebyte(0x10); // HTYACinfo
	for (i = 0; i < 16; i++)
		writebyte(std_ac_luminance_nrcodes[i+1]);
	for (i = 0; i < 162; i++)
		writebyte(std_ac_luminance_values[i]);
	
	writebyte(1); // HTCbDCinfo
	for (i = 0; i < 16; i++)
		writebyte(std_dc_chrominance_nrcodes[i+1]);
	for (i = 0; i < 12; i++)
		writebyte(std_dc_chrominance_values[i]);

	writebyte(0x11); // HTCbACinfo = 0x11;
	for (i = 0; i < 16; i++)
		writebyte(std_ac_chrominance_nrcodes[i+1]);
	for (i = 0; i < 162; i++)
		writebyte(std_ac_chrominance_values[i]);

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
static void writebits(bitbuffer_t *const pbb, unsigned bits, unsigned nbits)
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
		if (b == 0xFF) // replace 0xFF with 0xFF00
			writebyte(0);
	}

	pbb->n = nbits;
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
static void flushbits(bitbuffer_t *pbb)
{
	if(pbb->n != 0)
		writebits(pbb, 0xFFu, 8-pbb->n);
}

/******************************************************************************
**  huffman_bits
**  --------------------------------------------------------------------------
**  Converst amplitude into the representation suitable for Huffman encoder.
**  
**  ARGUMENTS:
**      value    - DCT amplitude;
**
**  RETURN: huffman bits
******************************************************************************/
static unsigned huffman_bits(const short value)
{
	return value - ((unsigned)value >> 31);
}

/******************************************************************************
**  huffman_magnitude
**  --------------------------------------------------------------------------
**  Calculates magnitude of an amplitude - the number of bits that are enough
**  to represent given value.
**  
**  ARGUMENTS:
**      value    - DCT amplitude;
**
**  RETURN: magnitude
******************************************************************************/
static unsigned huffman_magnitude(const short value)
{
	unsigned x = (value < 0)? -value: value;
	unsigned m = 0;

	while (x >> m) ++m;

	return m;
}
/******************************************************************************
**  gen_table
**  --------------------------------------------------------------------------
**  Generate the quantizzation table depending on choosen quality
**  
**  ARGUMENTS:
**     	q_fctr	- quality factor
**
**  RETURN: -
******************************************************************************/
void gen_table(unsigned q_fctr)
{
    unsigned i,j,val,s,std_val; 
    if (q_fctr < 50) s = 5000 / q_fctr;   
    else s = 200 - 2 * q_fctr;           
    for(i = 0; i <= 7; i++){            
        for(j = 0; j <= 7; j++){
			std_val = (unsigned)std_q_table[i][j];
            val = (unsigned) round((s*std_val + 50)/100);
            //check if is between 1 and 255
            if (val > 255) val = 255;
            if (val < 1) val = 1;
            q_table[i][j] = (unsigned char) round(val);
		}
    }   
}

/******************************************************************************
**  huffman_start
**  --------------------------------------------------------------------------
**  Starts Huffman encoding by writing Start of Image (SOI) and all headers.
**  Sets image size in Start of File (SOF) header before writing it.
**  
**  ARGUMENTS:
**      height  - image height (pixels);
**      width   - image width (pixels);
**	image_type_t - image type;
**	res - image resolution;
**
**  RETURN: -
******************************************************************************/
char huffman_start(short height, short width, image_type_t t, unsigned q_fctr)
{
    bitbuf.buf = 0;
    bitbuf.n = 0;

    //Generate the appropriate quantization table depending on selected compression level
    gen_table(q_fctr);
    huffman_ctx[0].qtable = q_table;
    
    char tmp = 0;
    writeword(0xFFD8); // SOI
    write_APP0info();
    write_DQTinfo();
    tmp = write_SOF0info(height, width, t);
    if (tmp < 0)
	    return tmp;
    write_DHTinfo();
    tmp = write_SOSinfo(t);
    if (tmp < 0)
	    return tmp;
    huffman_ctx[2].dc = 
    huffman_ctx[1].dc = 
    huffman_ctx[0].dc = 0;
    return 0;
}

/******************************************************************************
**  huffman_stop
**  --------------------------------------------------------------------------
**  Finalize Huffman encoding by flushing bit-buffer, writing End of Image (EOI)
**  into output buffer and flusing this buffer.
**  
**  ARGUMENTS: -
**
**  RETURN: -
******************************************************************************/
void huffman_stop(void)
{
	flushbits(&bitbuf);
	writeword(0xFFD9); // EOI - End of Image
	write_jpeg(jpgbuff, jpgn);
	jpgn = 0;
}

/******************************************************************************
**  huffman_encode

**  --------------------------------------------------------------------------
**  Quantize and Encode a 8x8 DCT block by JPEG Huffman lossless coding.
**  This function writes encoded bit-stream into bit-buffer.
**  
**  ARGUMENTS:
**      ctx     - pointer to encoder context;
**      data    - pointer to 8x8 DCT block;
**
**  RETURN: -
******************************************************************************/
void huffman_encode(huffman_t *const ctx, const short data[])
{
	unsigned magn, bits;
	unsigned zerorun, i;
	short    diff;
	short    dc = data[0];

	// difference between old and new DC
	diff = dc - ctx->dc;
	ctx->dc = dc;

	bits = huffman_bits(diff);
	magn = huffman_magnitude(diff);

	writebits(&bitbuf, ctx->hdcbit[magn], ctx->hdclen[magn]);
	writebits(&bitbuf, bits, magn);

	for (zerorun = 0, i = 1; i < 64; i++)
	{
		const short ac = data[i]; /*Zinzipeg change: no quantization*/

		if (ac) {
			while (zerorun >= 16) {
				zerorun -= 16;
				// ZRL
				writebits(&bitbuf, ctx->hacbit[15][0], ctx->haclen[15][0]);
			}

			bits = huffman_bits(ac);
			magn = huffman_magnitude(ac);

			writebits(&bitbuf, ctx->hacbit[zerorun][magn], ctx->haclen[zerorun][magn]);
			writebits(&bitbuf, bits, magn);

			zerorun = 0;
		}
		else zerorun++;
	}

	if (zerorun) { // EOB - End Of Block
		writebits(&bitbuf, ctx->hacbit[0][0], ctx->haclen[0][0]);
	}

}
