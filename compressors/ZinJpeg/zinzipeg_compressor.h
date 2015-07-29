#ifndef __zinzipeg_compressor_h__
#define __zinzipeg_compressor_h__
#include "compressor.h"

class zinzipeg_compressor : public compressor{
	
public :
	void setSize( int width, int height);
	int compress(unsigned char *buffer_in,unsigned char *buffer_out);
	void decompress(unsigned char *buffer_in,unsigned char *buffer_out);
	void setQuality(int quality);
	void setMarkers(int markers_length);

private :
	char *buffer;
	int width;
	int height;
	int compressed_size;

};

#endif
