#pragma once

#include "stdint_.h"

int base32_decode(const uint8_t *encoded, uint8_t *result, int bufSize)
    __attribute__((visibility("hidden")));
int base32_encode(const uint8_t *data, int length, uint8_t *result,
                  int bufSize)
    __attribute__((visibility("hidden")));