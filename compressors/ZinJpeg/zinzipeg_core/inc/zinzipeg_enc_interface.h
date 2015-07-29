#ifndef ZINZIPEG_INTERFACE
#define ZINZIPEG_INTERFACE

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Sets the quality of the JPEG compression.
 * Must be between 1 and 100 (included).
 * Values out of range are automatically set to the closest valid value.
 *
 * Default is 75
 */
void zinzipeg_set_quality(unsigned char q);

/**
 * Sets the marker distance.
 * Must be greater or equal than 1.
 * Values out of range results in undefined behavior.
 *
 * Default is 5
 */
void zinzipeg_set_distance(unsigned char d);

/**
 * Sets the image width and height in pixels.
 * Both must be multiple of 8, otherwise undefined behavior may occur.
 *
 * Default is 160x120
 */
void zinzipeg_set_size(unsigned short x, unsigned short y);


/**
 * Encodes an image.
 *
 * The input parameter must be of exactly (image width x image
 * height) bytes (1 byte per pixel, grayscale).
 *
 * The output buffer does not have a maximum size, because it heavily
 * depends on how well the image compress. Note that the (width x
 * height) bytes might be not enough. It is recommended to use a
 * buffer of at least 4 times the raw image.
 *
 * It returns the number of bytes written to the output buffer.
 */
int zinzipeg_encode(unsigned char* in_buf, unsigned char* out_buf);

/**
 * Returns the total number of bits in the payload of the packets,
 * excluding the overhead of padding and the trailers.
 *
 * The statistics are computed on all the images encoded since the
 * last reset occurred.
 */
unsigned zinzipeg_get_stats();

/**
 * Returns the total size of all images (in bits), including the
 * overhead of padding and the trailers.
 *
 * The statistics are computed on all the images encoded since the
 * last reset occurred.
 */
unsigned zinzipeg_get_overhead_stats();

/**
 * Resets the encoding statistics.
 */
void zinzipeg_reset_stats();

#ifdef __cplusplus
}
#endif

#endif
