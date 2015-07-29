#ifndef __DCT_H__
#define __DCT_H__

#ifdef __cplusplus
extern "C" {
#endif

// integer DCTs
void dct(unsigned char *pixel, short *data);
void dct3(unsigned char *pixel, short *data);

#ifdef __cplusplus
}
#endif

#endif//__DCT_H__
