#include <stdio.h>
#include <stdint.h>
#include "zinjpeg_config.hh"


int packet_num = 0;

static void arrange(uint64_t data, uint64_t *out) {
	*out = data & 0x7f;
	data = data << 1;
	*out |= data & 0x7fff00;
	data = data << 1;
	*out |= data & 0x7f000000;
	*out |= (data << 1) & 0x0700000000l;
}

static void arrange_array(uint64_t *data, uint8_t *t) {
	for (int i = 0; i < 40; i++)
		t[i] = (*data >> (39 - i)) & 0x01;
}

static void arrange_output(uint64_t *out, uint8_t *t) {
	if (t[0] == 1)
		(*out) |= 0x8000000000l;
	if (t[1] == 1)
		(*out) |= 0x4000000000l;
	if (t[2] == 1)
		(*out) |= 0x2000000000l;
	if (t[4] == 1)
		(*out) |= 0x0800000000l;
	if (t[8] == 1)
		(*out) |= 0x0080000000l;
	if (t[16] == 1)
		(*out) |= 0x0000800000l;
	if (t[32] == 1)
		(*out) |= 0x80l;
}

void SECDED_encoder(uint32_t data, uint64_t *out) {
	// arrange the data leaving blank spaces for parity bit
	arrange(data, out);
	uint8_t tmp[40];
	// put in tmp array the data
	arrange_array(out, tmp);
	//setting p0
	tmp[1] = tmp[3]^tmp[5]^tmp[7]^tmp[9]^
				tmp[11]^tmp[13]^tmp[15]^tmp[17]^
				tmp[19]^tmp[21]^tmp[23]^tmp[25]^
				tmp[27]^tmp[29]^tmp[31]^tmp[33]^
				tmp[35]^tmp[37]^tmp[39];
	//setting p1
	tmp[2] = tmp[3]^tmp[6]^tmp[7]^tmp[10]^
				tmp[11]^tmp[14]^tmp[15]^tmp[18]^
				tmp[19]^tmp[22]^tmp[23]^tmp[26]^
				tmp[27]^tmp[30]^tmp[31]^tmp[34]^
				tmp[35]^tmp[38]^tmp[39];
	//setting p2
	tmp[4] = tmp[5]^tmp[6]^tmp[7]^tmp[12]^
				tmp[13]^tmp[14]^tmp[15]^tmp[20]^
				tmp[21]^tmp[22]^tmp[23]^tmp[28]^
				tmp[29]^tmp[30]^tmp[31]^tmp[36]^
				tmp[37]^tmp[38]^tmp[39];
	//setting p3
	tmp[8] = tmp[9]^tmp[10]^tmp[11]^tmp[12]^
				tmp[13]^tmp[14]^tmp[15]^tmp[24]^
				tmp[25]^tmp[26]^tmp[27]^tmp[28]^
				tmp[29]^tmp[30]^tmp[31];
	// setting p4
	tmp[16] = tmp[17]^tmp[18]^tmp[19]^tmp[20]^
				tmp[21]^tmp[22]^tmp[23]^tmp[24]^
				tmp[25]^tmp[26]^tmp[27]^tmp[28]^
				tmp[29]^tmp[30]^tmp[31];
	// setting p5
	tmp[32] = tmp[33]^tmp[34]^tmp[35]^tmp[36]^
				tmp[37]^tmp[38]^tmp[39];
	//setting overall parity bit
	tmp[0] = tmp[1]^tmp[2]^tmp[3]^tmp[4]^
				tmp[5]^tmp[6]^tmp[7]^tmp[8]^
				tmp[9]^tmp[10]^tmp[11]^tmp[12]^
				tmp[13]^tmp[14]^tmp[15]^tmp[16]^
				tmp[17]^tmp[18]^tmp[19]^tmp[20]^
				tmp[21]^tmp[22]^tmp[23]^tmp[24]^
				tmp[25]^tmp[26]^tmp[27]^tmp[28]^
				tmp[29]^tmp[30]^tmp[31]^tmp[32]^
				tmp[33]^tmp[34]^tmp[35]^tmp[36]^
				tmp[37]^tmp[38]^tmp[39];

	arrange_output(out, tmp);
}

static uint8_t check_parity(uint64_t *data) {
	uint8_t out = 0;
	for (int i = 0; i < 40; i++)
		out = out ^ ((*data >> i) & 0x01);
	return out;
}

static uint8_t get_syndrome(uint64_t *data) {
	uint8_t out = (*data >> 38) & 0x01;
	out |= (*data >> 36) & 0x02;
	out |= (*data >> 33) & 0x04;
	out |= (*data >> 28) & 0x08;
	out |= (*data >> 19) & 0x10;
	out |= (*data >> 2) & 0x20;
	return out;
}

static unsigned extract(uint64_t *data) {
	unsigned out = *data & 0x7f;
	out |= (*data & 0x7fff00) >> 1;
	out |= (*data & 0x007f000000l) >> 2;
	out |= (*data & 0x0700000000l) >> 3;
	return out;
}

static void recover(uint64_t *data, uint8_t syndr) {
	*data ^= 0x8000000000l >> syndr;
}


short safety_level;
//! Returns the current safety level (see SECDED_decoder)
short get_safety_level() {
	return safety_level;
}

/**
 * This function is an hammer decoder.
 * It sets safety_level to:
 * - 0 if no error detected
 * - 1 if an error has been detected and corrected
 * - 2 if undetectable errors have been detected
 * @param[in] rdata contains the trailer to analyze
 * @return the corrected data
 */
uint64_t SECDED_decoder(uint64_t *rdata) {
	// Bits 3 is reserved and its value must not be considered
	// to avoid to consider bit flips in that positions, we clear it
	*rdata &= 0xefffffffffl;
	// Start of the proper decoding
 	uint8_t error = check_parity(rdata);
	uint64_t extracted = extract(rdata);
	DEBUG_HAMMING_VALUES(packet_num, extracted, *rdata);
	uint64_t tdata;
	SECDED_encoder((unsigned)extracted, &tdata);
	uint8_t syndr = get_syndrome(rdata) ^ get_syndrome(&tdata);
	DEBUG_HAMMING_SYNDR(syndr);
	// if no error occurred
	if (syndr == 0) {
		safety_level = 0;
		return extracted;
	}
	// if uncorrectable error
	if (error == 0) {
		DEBUG_HAMMING("Uncorrectable error detected");
		safety_level = 2;
		return extracted;
	}
	// if correctable error
	DEBUG_HAMMING("Detected recoverable error");
	safety_level = 1;
	recover(rdata, syndr);
	return extract(rdata);
}



/*
int main(void) {
	unsigned i;
	unsigned flip;
	while(1) {
		printf("\nEnter the number to encode: ");
		scanf("%u", &i);
		i = SECDED_encoder(i);
		printf("Encoded data: %u\n", i);
		printf("Enter the position of a bit to flip: ");
		scanf("%u", &flip);
		if (flip < 32) i = recover(i, flip);
		printf("Another bit flip?: ");
		scanf("%u", &flip);
		if (flip < 32) i = recover(i, flip);
		printf("Corrupted data: %u\n", i);
		i = SECDED_decoder(i);
		printf("Decoded data: %u\n", i);
	}
	return 0;
}*/
