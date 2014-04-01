#pragma once
#include "stdint_.h"
#include <time.h>

enum totpType_t {
  totpGoogle = 0,
  totpBattlenet = 1,
  totpTypeCount
};

struct totpInfo_t {
  uint8_t seconds;
  uint8_t digit;
  uint8_t offset;
  const char *format;
};

struct otpInfo_t {
  enum totpType_t func;
  uint16_t secretLength;
  uint8_t *secret;
  char *description;
  char *currentCode;
  uint16_t remainTime;
};

struct otpInfoList_t {
  uint32_t count;
  struct otpInfo_t *infos;
};

void generateCode(struct otpInfo_t *otpInfo, const time_t currentTime);
uint8_t getOTPCodeLength(const enum totpType_t type);

extern struct totpInfo_t totpInfo[totpTypeCount];
extern struct otpInfoList_t otpInfoList;