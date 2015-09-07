#ifndef REF_ENCDEC_H
#define REF_ENCDEC_H

#define DECODER_ONLY 1

uint8_t golay_decode(uint32_t *cw, uint8_t *errs);
//uint32_t syndrome(uint32_t cw);

#if (DECODER_ONLY == 0)
uint32_t golay_encode(uint32_t cw);
#endif

#endif