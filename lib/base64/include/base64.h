#ifndef BASE64_H
#define BASE64_H
#include <cstdint>
#include <cstddef>

char *encode_base64(const uint8_t *in, size_t len);
uint8_t *decode_base64(const char *in, size_t len, size_t *olen);

#endif // BASE64_H
