#ifndef _CONFIG_
#define _CONFIG_

//! Width of the images to process (pixels)
#define IMAGE_WIDTH         640
//static int IMAGE_WIDTH = 640;
//! Height of the images to process (pixels)
#define IMAGE_HEIGHT        480
//static int IMAGE_HEIGHT = 480;
//! Size of JPEG blocks (pixels)
#define BLOCK_SIZE          8

//! Size of a frame (bytes)
#define PCKT_DIM 104

//! Trailer size (bytes)
#define TRAILER_SIZE 5

//! PDU lenght (bytes)
#define MAX_PDU_LENGTH_B	(PCKT_DIM - TRAILER_SIZE)

//! PDU lenght (bits)
#define MAX_PDU_LENGTH MAX_PDU_LENGTH_B *8

/**
 * DEBUG_MAIN
 * Enables logging about errors which are catched in the upper
 * level of the program.
 * Default: 1
 */
#define	DEBUG_MAIN_ENABLED	1

/**
 * DEBUG_DC
 * Prints the DC values found by the Huffman decoder
 * Default: 0
 */
#define	DEBUG_DC_ENABLED	0

/**
 * DEBUG_AC
 * Prints the AC values found by the Huffman decode
 * Default: 0
 */
#define	DEBUG_AC_ENABLED	0

/**
 * DEBUG_MATCHES
 * Prints all the matches found in the huffman decoder module
 * Default: 0
 */
#define DEBUG_MATCHES_ENABLED	0

/**
 * DEBUG_EOB
 * Prints when found a EOB marker
 * Default: 0
 */
#define DEBUG_EOB_ENABLED	0

/**
 * Prints information about the Hamming decoding process
 * Default: 1
 */
#define DEBUG_HAMMING_ENABLED	1

/**
 * If 1 prints informations on all the bytes read in the read_from_file
 * function (defined in main.cc)
 * NOTE: this is very very verbose
 * Default: 0
 */
#define	DEBUG_DATA_ENABLED 0

/**
 * Prints the final destination of a given block (skipped or accepted)
 * Default: 1
 */
#define DEBUG_BLOCK_ENABLED 1

/**
 * Prints information on the pkg_reader module, such as the loading 
 * of a new packet, the data contained in its trailer...
 * Default: 1
 */
#define DEBUG_PACKET_ENABLED 1

/**
 * Prints information about the action performed by the trailer_checker
 * Default: 1
 */
#define DEBUG_TRAILER_ENABLED 1

/**
 * Prints when a block is actually written in the output buffer
 * Default: 0
 */
#define DEBUG_ENCODER_ENABLED 0


/**********************************************************/
/* DO NOT MODIFY UNDER THIS LINE **************************/
/**********************************************************/
#define NUM_COEFF			BLOCK_SIZE * BLOCK_SIZE
#define IMAGE_SIZE          (IMAGE_WIDTH * IMAGE_HEIGHT)
//static int IMAGE_SIZE = (IMAGE_HEIGHT*IMAGE_WIDTH);
#define NUM_OF_BLOCKS 		(IMAGE_SIZE / 64)
//static int NUM_OF_BLOCKS = (IMAGE_SIZE/64);
//#define NUM_OF_BLOCKS 		4800

#if DEBUG_ENCODER_ENABLED
	#define DEBUG_ENC(x)	printf("[ENCODER]    Writing block %d\n", x)
#else
	#define DEBUG_ENC(x)
#endif

#if DEBUG_TRAILER_ENABLED
	#define DEBUG_TC(x)		printf("[TRAILER]    %s\n", x)
	#define DEBUG_TC_DATA(x,w,y,z) \
		printf("[TRAILER]    Alignment: %d\n[TRAILER]    Flag coherence: %d (prev: %d, new: %d)\n",x,w,y,z)
#else
	#define DEBUG_TC(x)
	#define DEBUG_TC_DATA(x,y,w,z)
#endif

#if DEBUG_PACKET_ENABLED
	#define DEBUG_PACKET(x)		printf("[PKG_READER] %s\n", x)
	#define DEBUG_PACKET_2(x,y)	printf("[PKG_READER] %s%d\n", x, y)
	#define DEBUG_PACKET_3(x,y,w,z) printf("[PKG_READER] fl:%d  lBlck:%d  #Blck:%d  #b:%d\n",x,y,w,z)
#else 
	#define DEBUG_PACKET(x)
	#define DEBUG_PACKET_2(x,y)
	#define DEBUG_PACKET_3(x,y,w,z)
#endif

#if DEBUG_HAMMING_ENABLED
	#define DEBUG_HAMMING(x) printf("[HAMMING]    %s\n", x)
	#define DEBUG_HAMMING_SYNDR(x) printf("[HAMMING]    Syndrome is: %u\n", x)
	#define DEBUG_HAMMING_VALUES(n,x,y)	printf("[HAMMING]    #%d\textr:%lx\trecv:%lx\n", n, x, y)
#else
	#define DEBUG_HAMMING(x)
	#define DEBUG_HAMMING_SYNDR(x)
	#define DEBUG_HAMMING_VALUES(n,x,y)
#endif

#if DEBUG_MAIN_ENABLED
	#define DEBUG_MAIN(x)	printf("[MAIN]       %s\n", x)
#else
	#define DEBUG_MAIN(x)
#endif

#if DEBUG_DATA_ENABLED
	#define DEBUG_DATA(x,y)	printf("[READ]       length=%d    data=%d\n", x, y)
#else
	#define DEBUG_DATA(x,y)
#endif

#if DEBUG_BLOCK_ENABLED
	#define DEBUG_BLOCK(x,y) printf("[BLOCK]      #%d: %s\n", x, y)
#else
	#define DEBUG_BLOCK(x,y)
#endif

#if DEBUG_DC_ENABLED
#define DEBUG_DC(x,y,z)		printf("[HuffDC]     DC coefficient of %d bits: %d",x,y); \
							printf(" (original %d) \n",z);
#else
#define DEBUG_DC(x,y,z)
#endif

#if DEBUG_AC_ENABLED
#define DEBUG_AC(x,y,z)		printf("[HuffAC]     AC coefficient of %d bits, %d zerorun : %d \n",x,y,z);
#else
#define DEBUG_AC(x,y,z)
#endif

#if DEBUG_MATCHES_ENABLED
#define DEBUG_MATCHES(x,y,z)	printf(x);	\
								printbits(y, z);\
								printf("\n")
#else
#define DEBUG_MATCHES(x,y,z)
#endif

#if DEBUG_EOB_ENABLED
#define DEBUG_EOB(x)	printf("[HUFFMAN]    EOB at coefficient %d\n",x);
#else
#define DEBUG_EOB(x)
#endif

#if !defined(CONCEAL_BLACK) && !defined(CONCEAL_COPY) && !defined(CONCEAL_GRAY)
#define CONCEAL_GRAY
#warning IMPORTANT! You did not specify the type of concealment! I am going to defalt it to -DGRAY_CONCEALMENT!
#endif

#endif // _CONFIG_
