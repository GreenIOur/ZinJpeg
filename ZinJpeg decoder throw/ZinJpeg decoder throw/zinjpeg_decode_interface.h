//
//  zinjpeg_decode_interface.h
//  ZinJpeg decoder throw
//
//  Created by nicolas dagieu on 29/07/2015.
//  Copyright (c) 2015 nicolas dagieu. All rights reserved.
//

#ifndef __ZinJpeg_decoder_throw__zinjpeg_decode_interface__
#define __ZinJpeg_decoder_throw__zinjpeg_decode_interface__

#include <stdio.h>

class zinjpeg_decompressor{
    
    private :
    
        int marker_length;
        int quality;
        int input_buffer_size;
        int output_size;
        unsigned char* output_buffer;
    
    
    public :
    
        zinjpeg_decompressor();
        zinjpeg_decompressor(int quality = 75, int marker_length = 5);
        ~zinjpeg_decompressor();
        void init_compressor(int quality = 75, int marker_length = 5);
        unsigned char* decompress_zinjpeg(unsigned char* input_buffer, int buffer_size);
        int getMarker_length(void);
        int getQuality(void);
        int getSize(void);
        unsigned char* getOut_buffer(void);
    
    
};

#endif /* defined(__ZinJpeg_decoder_throw__zinjpeg_decode_interface__) */
