#pragma once

#include "stdint_.h"

void hmac_sha1(const uint8_t *key, int keyLength,
               const uint8_t *data, int dataLength,
               uint8_t *result, int resultLength)
 __attribute__((visibility("hidden")));