#pragma once

#include "stdint_.h"

#define SHA1_BLOCKSIZE     64
#define SHA1_DIGEST_LENGTH 20

typedef struct {
  uint32_t digest[8];
  uint32_t count_lo, count_hi;
  uint8_t  data[SHA1_BLOCKSIZE];
  int      local;
} SHA1_INFO;

void sha1_init(SHA1_INFO *sha1_info) __attribute__((visibility("hidden")));
void sha1_update(SHA1_INFO *sha1_info, const uint8_t *buffer, int count)
  __attribute__((visibility("hidden")));
void sha1_final(SHA1_INFO *sha1_info, uint8_t digest[20])
  __attribute__((visibility("hidden")));