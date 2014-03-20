#include "stdint_.h"
#include "totp.h"
#include "pebble_os.h"
#include "base32.h"
#include "hmac.h"
#include "sha1.h"
#include <time.h>
#include <string.h>

float TIMEZONE_MAP[40] = {
  -12, -11, -10, -9.5, -9, -8, -7, -6, -5, -4.5, -4, -3.5, -3, -2, -1, 0, 1, 2, 3,
  3.5, 4, 4.5, 5, 5.5, 5.75, 6, 6.5, 7, 8, 8.75, 9, 9.5, 10, 10.5, 11, 11.5, 12, 12.75, 13, 14};
uint8_t TIMEZONE = 30;

totpInfo_t totpInfo[totpTypeCount] = {
  { // google
    .seconds = 30,
    .digit = 6,
    .offset = SHA1_DIGEST_LENGTH - 1
  },
  { // battlenet
    .seconds = 30,
    .digit = 8,
    .offset = 19
  }
};

uint32_t generateCode(const char *secret, totpType_t type, int8_t *remainTime) {
  uint8_t key[100] = {0};
  uint16_t keyLen = base32_decode((const uint8_t *)secret, key, 100);
  uint64_t t = time(NULL) - (int64_t)(TIMEZONE_MAP[TIMEZONE] * 60 * 60);
  uint64_t value = t / totpInfo[(uint8_t)type].seconds;
  uint8_t val[8] = {0};
  for (int i = 8; i--; value >>= 8) {
    val[i] = value;
  }
  uint8_t hash[SHA1_DIGEST_LENGTH];
  hmac_sha1(key, keyLen, val, 8, hash, SHA1_DIGEST_LENGTH);
  int offset = hash[totpInfo[(uint8_t)type].offset] & 0xF;
  unsigned int truncatedHash = 0;
  for (int i = 0; i < 4; ++i) {
    truncatedHash <<= 8;
    truncatedHash  |= hash[offset + i];
  }
  memset(hash, 0, sizeof(hash));
  truncatedHash &= 0x7FFFFFFF;
  uint32_t digit = 1;
  for(uint8_t i = 0; i < totpInfo[(uint8_t)type].digit; i++) {
    digit *= 10;
  }
  truncatedHash %= digit;
  *remainTime = -(t % totpInfo[(uint8_t)type].seconds - totpInfo[(uint8_t)type].seconds);
  return truncatedHash;
}