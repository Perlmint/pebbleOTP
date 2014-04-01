#include <pebble.h>
#include "totp.h"
#include "persistant.h"
#include "otpWindow.h"
#include "util.h"

static void in_received_handler(DictionaryIterator *received, void *context) {
}

static void in_dropped_handler(AppMessageResult reason, void *context) {
}

static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
}

void tickHandler(struct tm *tick_time, TimeUnits units_changed) {
  app_log(APP_LOG_LEVEL_DEBUG_VERBOSE, __FILE__, __LINE__, "tick in, %lu", (uint32_t)tick_time);
  static time_t lastUpdate = 0;
  time_t curTime = UTIL_FUNC(mktime)(tick_time);
  int16_t timeDelta = curTime - lastUpdate;
  
  app_log(APP_LOG_LEVEL_DEBUG_VERBOSE, __FILE__, __LINE__, "tick (%lu)", curTime);
  uint16_t i = 0;
  for (; i < otpInfoList.count; i++) {
    if (otpInfoList.infos[i].remainTime <= timeDelta) {
      generateCode(&otpInfoList.infos[i], curTime);
    }
    else {
      otpInfoList.infos[i].remainTime -= timeDelta;
    }
  }
  
  lastUpdate = curTime;
  updateOTPWindow();
}

void init() {
  // Load OTP Info from Persist storage
  loadOTPInfoList(&otpInfoList);
  openOTPWindow();
  tick_timer_service_subscribe(SECOND_UNIT, tickHandler);
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_failed(out_failed_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

void deinit() {
  tick_timer_service_unsubscribe();
  app_message_deregister_callbacks();
  //window_stack_pop_all(true);
  deinitOTPWindow();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
