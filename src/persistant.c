#include <pebble.h>
#include "persistant.h"
#include "totp.h"

void loadOTPInfoList(struct otpInfoList_t *info) {
  app_log(APP_LOG_LEVEL_DEBUG_VERBOSE, __FILE__, __LINE__, "load otp info");
  if (!persist_exists(SAVED_OTP_COUNT)) {
    app_log(APP_LOG_LEVEL_DEBUG_VERBOSE, __FILE__, __LINE__, "otp notfound");
    return;
  }
  
  uint32_t i;
  info->count = persist_read_int(SAVED_OTP_COUNT);

  if (info->infos != NULL) {
    free(info->infos);
  }
  info->infos = (struct otpInfo_t *)malloc(sizeof(struct otpInfo_t) * info->count);

  for (i = 0; i < info->count; ++i) {
    struct otpInfo_t *otpInfoItem = &info->infos[i];
    otpInfoItem->func = persist_read_int(i * 4 + 2);

    otpInfoItem->secretLength = persist_read_int(i * 4 + 2 + 1);

    otpInfoItem->secret = (uint8_t *)malloc(sizeof(uint8_t) * otpInfoItem->secretLength);
    persist_read_data(i * 4 + 2 + 2, otpInfoItem->secret, otpInfoItem->secretLength);

    otpInfoItem->description = (char *)malloc(sizeof(char) * 20);
    persist_read_string(i * 4 + 2 + 3, otpInfoItem->description, 20);

    otpInfoItem->remainTime = 0;

    otpInfoItem->currentCode = (char *)malloc(sizeof(char) * (getOTPCodeLength(otpInfoItem->func) + 1));
    otpInfoItem->currentCode[0] = '\0';
  }
}
