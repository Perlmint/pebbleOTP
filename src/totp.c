#include "stdint_.h"
#include "totp.h"
#include "hmac.h"
#include "sha1.h"
#include <time.h>
#include <string.h>
#include <stdio.h>
  
struct otpInfoList_t otpInfoList = {
  .count = 0,
  .infos = NULL
};

struct totpInfo_t totpInfo[totpTypeCount] = {
  { // google
    .seconds = 30,
    .digit = 6,
    .offset = SHA1_DIGEST_LENGTH - 1,
    .format = "%06ul"
  },
  { // battlenet
    .seconds = 30,
    .digit = 8,
    .offset = 19,
    .format = "%08ul"
  }
};

void generateCode(struct otpInfo_t *otpInfo, const time_t currentTime) {
  struct totpInfo_t *info = &totpInfo[(uint8_t)otpInfo->func];
  uint64_t t = currentTime;
  uint64_t value = t / info->seconds;
  uint8_t val[8] = {0};
  for (int i = 8; i--; value >>= 8) {
    val[i] = value;
  }
  uint8_t hash[SHA1_DIGEST_LENGTH];
  hmac_sha1(otpInfo->secret, otpInfo->secretLength, val, 8, hash, SHA1_DIGEST_LENGTH);
  int offset = hash[info->offset] & 0xF;
  unsigned int truncatedHash = 0;
  for (int i = 0; i < 4; ++i) {
    truncatedHash <<= 8;
    truncatedHash  |= hash[offset + i];
  }
  memset(hash, 0, sizeof(hash));
  truncatedHash &= 0x7FFFFFFF;
  uint32_t digit = 1;
  for(uint8_t i = 0; i < info->digit; i++) {
    digit *= 10;
  }
  truncatedHash %= digit;
  otpInfo->remainTime = -(t % info->seconds - info->seconds);
  snprintf(otpInfo->currentCode, info->digit + 1, info->format, truncatedHash);
}

uint8_t getOTPCodeLength(const enum totpType_t type) {
  return totpInfo[type].digit;
}