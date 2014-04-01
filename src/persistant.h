#pragma once

struct otpInfoList_t;

enum {
  SAVED_OTP_COUNT = 1
};

void loadOTPInfoList(struct otpInfoList_t *info);
