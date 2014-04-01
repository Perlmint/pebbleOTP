#include <pebble.h>
#include "totp.h"
#include "otpWindow.h"

static Window *window = NULL;
static TextLayer *descriptionLayer = NULL;
static TextLayer *codeLayer = NULL;
static int16_t selectedOTPIndex = -1;

void initOTPWindow(void) {
  app_log(APP_LOG_LEVEL_DEBUG_VERBOSE, __FILE__, __LINE__, "initWindow");
  // Setup UI
  descriptionLayer = text_layer_create(GRect(0, 0, 200, 100));
  codeLayer = text_layer_create(GRect(0, 100, 200, 100));
  Layer *rootLayer = window_get_root_layer(window);
  layer_add_child(rootLayer, (Layer *)descriptionLayer);
  layer_add_child(rootLayer, (Layer *)codeLayer);
}

Window *getOTPWindow(void) {
  app_log(APP_LOG_LEVEL_DEBUG_VERBOSE, __FILE__, __LINE__, "get window");
  if (window == NULL) {
    window = window_create();
    initOTPWindow();
  }
  return window;
}

void deinitOTPWindow(void) {
  if (window != NULL) {
    window_destroy(window);
    text_layer_destroy(descriptionLayer);
    text_layer_destroy(codeLayer);
  }
}

void openOTPWindow(void) {
  Window *otpWindow = getOTPWindow();
  Window *topWindow = window_stack_get_top_window();
  app_log(APP_LOG_LEVEL_DEBUG_VERBOSE, __FILE__, __LINE__, "open");
  if (topWindow != otpWindow && topWindow != NULL) {
    app_log(APP_LOG_LEVEL_DEBUG_VERBOSE, __FILE__, __LINE__, "pop");
    window_stack_pop(true);
  }
  if (!window_stack_contains_window(otpWindow)) {
    app_log(APP_LOG_LEVEL_DEBUG_VERBOSE, __FILE__, __LINE__, "push");
    window_stack_push(otpWindow, true);
  }
}

void updateOTPWindow(void) {
  if (selectedOTPIndex == -1) {
    text_layer_set_text(descriptionLayer, "Configured OTP");
    text_layer_set_text(codeLayer, "NOTFOUND");
  }
  else {
    text_layer_set_text(descriptionLayer, otpInfoList.infos[selectedOTPIndex].description);
    text_layer_set_text(codeLayer, otpInfoList.infos[selectedOTPIndex].currentCode);
  }
}
