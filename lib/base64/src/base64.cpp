#include "lib/base64/include/base64.h"

char *encode_base64(const uint8_t *in, size_t len)
{
    const char *dict = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
    size_t tail = len % 3;
    size_t count = len - tail;
    uint32_t en;
    char *out = new char[len << 1];
    size_t out_len = 0;
    for (size_t i = 0; i < count; i += 3, out_len += 4) {
        en = ((in[i] << 16) & 0xff0000) + ((in[i + 1] << 8) & 0xff00) + (in[i + 2] & 0xff);
        out[out_len] = dict[(en >> 18) & 0x3f];
        out[out_len + 1] = dict[(en >> 12) & 0x3f];
        out[out_len + 2] = dict[(en >> 6) & 0x3f];
        out[out_len + 3] = dict[en & 0x3f];
    }
    if (tail == 1) {
        en = in[len - 1];
        out[out_len++] = dict[en >> 2];
        out[out_len++] = dict[(en << 4) & 0x3f];
        out[out_len++] = '=';
        out[out_len++] = '=';
    } else if (tail == 2) {
        en = (in[len - 2] << 8) + in[len - 1];
        out[out_len++] = dict[en >> 10];
        out[out_len++] = dict[(en >> 4) & 0x3f];
        out[out_len++] = dict[(en << 2) & 0x3f];
        out[out_len++] = '=';
    }
    out[out_len] = 0;
    return out;
}

#define DICTPOS(k, v) \
if ((k) >= 'A' && (k) <= 'Z') { \
        v = (k) - 'A'; \
} else if ((k) >= 'a' && (k) <= 'z') { \
        v = (k) - 'a' + 26; \
} else if ((k) >= '0' && (k) <= '9'){ \
        v = (k) - '0' + 52; \
} else if ((k) == '+') { \
        v = 62; \
} else if ((k) == '/') { \
        v = 63; \
} else { \
        v = 64; \
}

uint8_t *decode_base64(const char *in, size_t len, size_t *olen)
{
    const char *dict = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
    size_t out_len = 0;
    size_t i, j;
    uint8_t c1, c2;
    *olen = 0;
    if ((len & 0x3) != 0) {
        return nullptr;
    }
    for (i = 0; i < len; i++) {
        for (j = 0; j < 65; j++) {
            if (in[i] == dict[j]) {
                break;
            }
        }
        if (j == 65) {
            return nullptr;
        }
    }
    uint8_t *out = new uint8_t[len];
    for (i = 0; i < len; i += 4) {
        DICTPOS(in[i], c1);
        DICTPOS(in[i + 1], c2);
        out[out_len++] = (c1 << 2) | (c2 >> 4);
        c1 = c2;
        DICTPOS(in[i + 2], c2);
        if (c2 != 64) {
            out[out_len++] = ((c1 & 0xf) << 4) | (c2 >> 2);
            c1 = c2;
            DICTPOS(in[i + 3], c2);
            if (c2 != 64) {
                out[out_len++] = ((c1 & 0x3) << 6) | c2;
            }
        }
    }
    *olen = out_len;
    return out;
}
