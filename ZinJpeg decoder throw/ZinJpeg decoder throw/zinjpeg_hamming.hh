#ifndef _SECDED_
#define _SECDED_

short get_safety_level();
uint64_t SECDED_decoder(uint64_t *rdata);
void SECDED_encoder(uint32_t data, uint64_t *out);

#endif