#ifndef REF_ENCDEC_H
#define REF_ENCDEC_H

uint8_t golay_decode(uint32_t *cw, uint8_t *errs);
uint32_t golay_encode(uint32_t cw);

#endif