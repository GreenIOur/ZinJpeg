//
//  zinjpeg_decode_interface.cpp
//  ZinJpeg decoder throw
//
//  Created by nicolas dagieu on 29/07/2015.
//  Copyright (c) 2015 nicolas dagieu. All rights reserved.
//

#include "zinjpeg_decode_interface.h"
#include "zinjpeg_encoder.hh"
#include "zinjpeg_huffman_decoder.hh"
#include "zinjpeg_pkg_reader.hh"
#include "stdlib.h"
#include "zinjpeg_errors.hh"
#include "zinjpeg_config.hh"
#include "zinjpeg_radio_stub.hh"
#include <cstring>
#include <cassert>
#include "zinjpeg_concealment.hh"
#include <iostream>
#include <stdlib.h>

/***************PRIVATE FUNCTIONS***********************//*******************************************************/
/*******************************************************//*******************************************************/

// Structure which contains the parameters passed at the program.
struct Parameters {
    unsigned quality;
    int marker_distance;
    std::ifstream in_file;
    char *out_file_name;
};
// Instance of struct parameters
struct Parameters params;


// This are used as static vars by the read_from_file
int last_data = 0;
int last_size = 0;


/**
 * Called from the bitbuffer module.
 * It reads bits from the current packet. An internal check is performed
 * to check whether we have read valid data or a marker.
 */
int read_from_file(unsigned short* out) {
    // If we saved data to read
    if(last_size != 0) {
        int tmp = last_size;
        (*out) = last_data;
        last_data = 0;
        last_size = 0;
        return tmp;
    }
    
    int x = 0, length = 0;
    length = pkg_reader_get(&x);
    DEBUG_DATA(length, x);
    
    // If there is the chance this is marker
    if(x == 255 && length == 8) {
        int next_data;
        length = pkg_reader_get(&next_data);
        // If we find an escape sequence, do nothing; otherwise
        // this is, in fact, a marker and we have to handle it.
        if(next_data != 0 || length != 8) {
            last_data = next_data;
            last_size = length;
            return ERR_MARKER;
        }
    }
    
    (*out) = x;
    return length;
}




//! This function is called whenever an anomaly situation occurs
void error_handler(short *coeff, errorcode_t retval, int *index){
    
    // Packet switch. Resync to the new first block value
    if (retval == ERR_PACKET_SWITCH){
        DEBUG_MAIN("A packet switch occurred");
        int new_index = get_first_block_index() - 1;
        int missing_blocks = new_index - written_blocks + 1;
        if (missing_blocks != 0)
            concealment_write_concealment_blocks(*index,missing_blocks);
        *index = new_index;
    }
    
    // No more packets are available. Fill the remaining part of the image
    else if (retval == ERR_EOF) {
        DEBUG_MAIN("EOF has been reached");
        concealment_write_concealment_blocks(*index, NUM_OF_BLOCKS - written_blocks);
        *index = NUM_OF_BLOCKS;
    }
    
    // Unexpected sequence during the Huffman decoding process. Sync on the next marker
    else if (retval == ERR_UNMATCHED_SEQUENCE) {
        DEBUG_MAIN("Unmatched Huffman's sequence detected. Resync on the next marker.");
        errorcode_t marker_reached = move_to_marker();
        // We may have reached the end of the image
        // or encountered other unexpected errors.
        if (marker_reached != 0) {
            error_handler(coeff, marker_reached, index);
            return;
        }
        int old_index = *index;
        // Resync to the next block to write
        *index += (params.marker_distance - *index % params.marker_distance-1);
        if(*index > NUM_OF_BLOCKS)
            *index = NUM_OF_BLOCKS - 1;
        concealment_write_concealment_blocks(old_index, *index - written_blocks + 1);
    }
    
    // Marker found. Reset the DC value so that the next one is read as absolute
    else if(retval == ERR_MARKER) {
        DEBUG_MAIN("Marker found. Next DC value is absolute");
        // Decrementing index. It has been incremented before, but what we read
        // was a marker, not a block
        *index -= 1;
        // Clear the bitbuffer, since it is now filled with garbage
        // and padding from last block
        bitbuffer_discard_bits(bitbuffer_get_size());
        // Reset the DC component.
        huff_reset();
    }
    
    // No other errors are supported
    else assert(false);
}

/**
 * This function ensure that we are not writing a block number
 * which goes over the last block contained in the current packet.
 */
void check_index(int *index){
    errorcode_t retval;
    short coefficients[NUM_COEFF];
    
    if ( (*index) > get_last_block_index() ) {
        // Discard the packet reading until it ends.
        // TODO: we might directly call the pkg_reader::load function
        // NOTE: i tried, but the result is different. Why?
        //do {
        while( (*index) > get_last_block_index() && retval != ERR_EOF) {
            DEBUG_BLOCK(*index, "Invalid. Skipping it (over current last_block value)");
            //load_frame();
            retval = read_block(coefficients);
        } //while (retval != ERR_PACKET_SWITCH && retval != ERR_EOF);
        
        // Since we have loaded a new packet, we have to sync to its first block
        concealment_write_concealment_blocks(*index, get_first_block_index() - *index);
        (*index) = get_first_block_index();
    }
    // if the first block value is wrong (less than expected) we have to sync,
    // skipping some blocks
    while (*index < written_blocks) {
        retval = read_block(coefficients);
        if(retval != 0) error_handler(coefficients, retval, index);
        (*index)++;
    }
}


//! Parses the parameters given to the program and store them in the structure params
void arg_parser(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Parameters: quality marker_rate in_file out_file\n");
        exit(1);
    }
    params.quality = (unsigned)atoi(argv[1]);
    params.marker_distance = atoi(argv[2]);
    params.in_file.open(argv[3]);
    params.out_file_name = argv[4];
}


/*******************************************************//*******************************************************/
/*******************************************************//*******************************************************/

zinjpeg_decompressor::zinjpeg_decompressor(){
    marker_length = 5;
    quality = 75;
    input_buffer_size = 0;
    output_buffer = NULL;
}

zinjpeg_decompressor::zinjpeg_decompressor(int quality, int marker_length){
    this->marker_length = marker_length;
    this->quality = quality;
    input_buffer_size = 0;
    output_buffer = NULL;
}

zinjpeg_decompressor::~zinjpeg_decompressor(){

}

void zinjpeg_decompressor::init_compressor(int quality, int marker_length){
    this->quality = quality;
    this->marker_length = marker_length;
}

unsigned char* zinjpeg_decompressor::decompress_zinjpeg(unsigned char* input_buffer, int buffer_size){
    
    //init of the decompression
    bitbuffer_construct();
    radio_init((char*)input_buffer, buffer_size);
    pkg_reader_construct();
    huff_table_init();
    huff_reset();
    concealment_initialize();
    
    //writting the header of the output buffer
    write_header(this->quality);
    
    
    //loop to transcode ZinJpeg in JPEG
    for (int i = 0; i < NUM_OF_BLOCKS; i++) {
        short coefficients[NUM_COEFF];
        // Check whether this index can be accepted
        //	check_index(&i);
        // Reading the block
        errorcode_t retval = read_block(coefficients);
        check_index(&i);
        
        if(retval == 0) {
            // i < NUM_OF_BLOCKS is necessary because we may encounter
            // a end of file in the check_index: in that case we sync
            // to the end of the image, but here we would try to write
            // one more block
            if(i < NUM_OF_BLOCKS) {
                DEBUG_BLOCK(i, "Valid block. I'm writing it");
                write_block(coefficients);
                concealment_block_written_success(i, coefficients);
            }
        } else error_handler(coefficients, retval, &i);
    }
    
    write_trailer();
    this->output_buffer = write_result_to_buffer(&(this->output_size));
    return this->output_buffer;
}

int zinjpeg_decompressor::getSize(void){
    return this->output_size;
}

unsigned char* zinjpeg_decompressor::getOut_buffer(void){
    return output_buffer;
}