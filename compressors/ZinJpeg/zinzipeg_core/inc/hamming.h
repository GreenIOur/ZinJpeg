#ifndef _SECDED_
#define _SECDED_

unsigned SECDED_decoder(uint64_t *rdata);
void SECDED_encoder(uint32_t data, uint64_t *trailer);

#endif