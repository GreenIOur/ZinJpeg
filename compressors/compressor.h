#ifndef __compressor_h__
#define __compressor_h__
#include <iostream>

class compressor{
	
public :

	virtual int compress(unsigned char *buffer_in,unsigned char *buffer_out) =0;
	virtual void decompress(unsigned char *buffer_in,unsigned char *buffer_out) =0;
	virtual void setQuality(int quality) =0;
	virtual void setMarkers(int markers_length) =0;
	virtual void setSize( int width, int height) =0;
	
};

#endif
