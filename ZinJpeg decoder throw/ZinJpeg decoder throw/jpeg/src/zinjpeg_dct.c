/*#include "arch.h"*/
#include "zinjpeg_dct.h"


/******************************************************************************
**  dct
**  --------------------------------------------------------------------------
**  Fast DCT - Discrete Cosine Transform.
**  This function converts 8x8 pixel block into frequencies.
**  Lowest frequencies are at the upper-left corner.
**  The input and output could point at the same array, in this case the data
**  will be overwritten.
**  
**  ARGUMENTS:
**      pixels  - 8x8 pixel array;
**      data    - 8x8 freq block;
**
**  RETURN: -
******************************************************************************/
void dct(unsigned char *pixels, short *data)
{
	short rows[64];
	unsigned i;

	static const int
				c1 = 1004,  /* cos(pi/16) << 10 */
				s1 = 200,   /* sin(pi/16) */
				c3 = 851,   /* cos(3pi/16) << 10 */
				s3 = 569,   /* sin(3pi/16) << 10 */
				r2c6 = 554, /* sqrt(2)*cos(6pi/16) << 10 */
				r2s6 = 1337,/* sqrt(2)*sin(6pi/16) << 10 */
				r2 = 181;   /* sqrt(2) << 7*/

	/* transform rows */
	for (i = 0; i < 8; i++)
	{
		int x0,x1,x2,x3,x4,x5,x6,x7,x8;

		x0 = pixels[i];
		x1 = pixels[8 + i];
		x2 = pixels[16 + i];
		x3 = pixels[24 + i];
		x4 = pixels[32 + i];
		x5 = pixels[40 + i];
		x6 = pixels[48 + i];
		x7 = pixels[56 + i];

		/* Stage 1 */
		x8=x7+x0;
		x0-=x7;
		x7=x1+x6;
		x1-=x6;
		x6=x2+x5;
		x2-=x5;
		x5=x3+x4;
		x3-=x4;

		/* Stage 2 */
		x4=x8+x5;
		x8-=x5;
		x5=x7+x6;
		x7-=x6;
		x6=c1*(x1+x2);
		x2=(-s1-c1)*x2+x6;
		x1=(s1-c1)*x1+x6;
		x6=c3*(x0+x3);
		x3=(-s3-c3)*x3+x6;
		x0=(s3-c3)*x0+x6;

		/* Stage 3 */
		x6=x4+x5;
		x4-=x5;
		x5=r2c6*(x7+x8);
		x7=(-r2s6-r2c6)*x7+x5;
		x8=(r2s6-r2c6)*x8+x5;
		x5=x0+x2;
		x0-=x2;
		x2=x3+x1;
		x3-=x1;

		/* Stage 4 and output */
		rows[i] =x6;
		rows[i + 32]=x4;
		rows[i + 16]=x8>>10;
		rows[i + 48]=x7>>10;
		rows[i + 56]=(x2-x5)>>10;
		rows[i + 8]=(x2+x5)>>10;
		rows[i + 24]=(x3*r2)>>17;
		rows[i + 40]=(x0*r2)>>17;
	}

	/* transform columns */
	for (i = 0; i < 8; i++)
	{
		int x0,x1,x2,x3,x4,x5,x6,x7,x8;

		x0 = rows[i * 8];
		x1 = rows[i * 8 + 1];
		x2 = rows[i * 8 + 2];
		x3 = rows[i * 8 + 3];
		x4 = rows[i * 8 + 4];
		x5 = rows[i * 8 + 5];
		x6 = rows[i * 8 + 6];
		x7 = rows[i * 8 + 7];

		/* Stage 1 */
		x8=x7+x0;
		x0-=x7;
		x7=x1+x6;
		x1-=x6;
		x6=x2+x5;
		x2-=x5;
		x5=x3+x4;
		x3-=x4;

		/* Stage 2 */
		x4=x8+x5;
		x8-=x5;
		x5=x7+x6;
		x7-=x6;
		x6=c1*(x1+x2);
		x2=(-s1-c1)*x2+x6;
		x1=(s1-c1)*x1+x6;
		x6=c3*(x0+x3);
		x3=(-s3-c3)*x3+x6;
		x0=(s3-c3)*x0+x6;

		/* Stage 3 */
		x6=x4+x5;
		x4-=x5;
		x5=r2c6*(x7+x8);
		x7=(-r2s6-r2c6)*x7+x5;
		x8=(r2s6-r2c6)*x8+x5;
		x5=x0+x2;
		x0-=x2;
		x2=x3+x1;
		x3-=x1;

		/* Stage 4 and output */
		data[i * 8]=((x6+16)>>3);
		data[i * 8 + 4]=((x4+16)>>3);
		data[i * 8 + 2]=((x8+16384)>>13);
		data[i * 8 + 6]=((x7+16384)>>13);
		data[i * 8 + 7]=((x2-x5+16384)>>13);
		data[i * 8 + 1]=((x2+x5+16384)>>13);
		data[i * 8 + 3]=(((x3>>8)*r2+8192)>>12);
		data[i * 8 + 5]=(((x0>>8)*r2+8192)>>12);
	}
}

// simple but fast DCT
void dct3(unsigned char *pixels, short *data)
{
	short rows[64];
	unsigned i;

	static const short // Ci = cos(i*PI/16)*(1 << 14);
		C1 = 16070,
		C2 = 15137,
		C3 = 13623,
		C4 = 11586,
		C5 = 9103,
		C6 = 6270,
		C7 = 3197;

	/* transform rows */
	for (i = 0; i < 8; i++)
	{
		short s07,s16,s25,s34,s0734,s1625;
		short d07,d16,d25,d34,d0734,d1625;

		s07 = pixels[i] + pixels[i + 56];
		d07 = pixels[i] - pixels[i + 56];
		s16 = pixels[i + 8] + pixels[i + 48];
		d16 = pixels[i + 8] - pixels[i + 48];
		s25 = pixels[i + 16] + pixels[i + 40];
		d25 = pixels[i + 16] - pixels[i + 40];
		s34 = pixels[i + 24] + pixels[i + 32];
		d34 = pixels[i + 24] - pixels[i + 32];

		rows[i + 8] = (C1*d07 + C3*d16 + C5*d25 + C7*d34) >> 14;
		rows[i + 24] = (C3*d07 - C7*d16 - C1*d25 - C5*d34) >> 14;
		rows[i + 40] = (C5*d07 - C1*d16 + C7*d25 + C3*d34) >> 14;
		rows[i + 56] = (C7*d07 - C5*d16 + C3*d25 - C1*d34) >> 14;

		s0734 = s07 + s34;
		d0734 = s07 - s34;
		s1625 = s16 + s25;
		d1625 = s16 - s25;

		rows[i] = (C4*(s0734 + s1625)) >> 14;
		rows[i + 32] = (C4*(s0734 - s1625)) >> 14;

		rows[i + 16] = (C2*d0734 + C6*d1625) >> 14;
		rows[i + 48] = (C6*d0734 - C2*d1625) >> 14;
	}

	/* transform columns */
	for (i = 0; i < 8; i++)
	{
		short s07,s16,s25,s34,s0734,s1625;
		short d07,d16,d25,d34,d0734,d1625;

		s07 = rows[i * 8] + rows[i * 8 + 7];
		d07 = rows[i * 8] - rows[i * 8 + 7];
		s16 = rows[i * 8 + 1] + rows[i * 8 + 6];
		d16 = rows[i * 8 + 1] - rows[i * 8 + 6];
		s25 = rows[i * 8 + 2] + rows[i * 8 + 5];
		d25 = rows[i * 8 + 2] - rows[i * 8 + 5];
		s34 = rows[i * 8 + 3] + rows[i * 8 + 4];
		d34 = rows[i * 8 + 3] - rows[i * 8 + 4];

		data[i * 8 + 1] = (C1*d07 + C3*d16 + C5*d25 + C7*d34) >> 16;
		data[i * 8 + 3] = (C3*d07 - C7*d16 - C1*d25 - C5*d34) >> 16;
		data[i * 8 + 5] = (C5*d07 - C1*d16 + C7*d25 + C3*d34) >> 16;
		data[i * 8 + 7] = (C7*d07 - C5*d16 + C3*d25 - C1*d34) >> 16;

		s0734 = s07 + s34;
		d0734 = s07 - s34;
		s1625 = s16 + s25;
		d1625 = s16 - s25;

		data[i * 8 ] = (C4*(s0734 + s1625)) >> 16;
		data[i * 8 + 4] = (C4*(s0734 - s1625)) >> 16;

		data[i * 8 + 2] = (C2*d0734 + C6*d1625) >> 16;
		data[i * 8 + 6] = (C6*d0734 - C2*d1625) >> 16;
	}
}
