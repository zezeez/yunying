/*
 * Copyright 2017-2021 The OpenSSL Project Authors. All Rights Reserved.
 * Copyright 2017 Ribose Inc. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#ifndef OSSL_CRYPTO_SM4_H
# define OSSL_CRYPTO_SM4_H
//# pragma once

//# include <openssl/opensslconf.h>
//# include <openssl/e_os2.h>
#include <stdint.h>
#include <QString>

# define SM4_ENCRYPT     1
# define SM4_DECRYPT     0

# define SM4_BLOCK_SIZE    16
# define SM4_KEY_SCHEDULE  32

typedef struct SM4_KEY_st {
    uint32_t rk[SM4_KEY_SCHEDULE];
} SM4_KEY;

const QString sm4_encrypt_ecb(const QString &plain, const QString &key_str);
const QString sm4_decrypt_ecb(const QString &crypt, const QString &key_str);

#endif
