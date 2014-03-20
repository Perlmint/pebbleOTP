#pragma once
#include "stdint_.h"

typedef enum {
  totpGoogle = 0,
  totpBattlenet = 1,
  totpTypeCount
} totpType_t;

typedef struct {
  uint8_t seconds;
  uint8_t digit;
  uint8_t offset;
} totpInfo_t;

uint32_t generateCode(const char *secret, totpType_t type, int8_t *remainTime);

extern totpInfo_t totpInfo[totpTypeCount];
extern float TIMEZONE_MAP[40];
extern uint8_t TIMEZONE;