#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "totp.h"

#define MY_UUID { 0x81, 0xEB, 0x27, 0xF8, 0x63, 0x69, 0x44, 0x7D, 0xBC, 0xFD, 0x84, 0x6B, 0xF9, 0x0F, 0x9B, 0x8A }
PBL_APP_INFO(MY_UUID,
             "POTP", "omniavinco",
             1, 0, /* App version */
             RESOURCE_ID_APP_ICON,
             APP_INFO_STANDARD_APP);

typedef struct {
  totpType_t type; // OTP Type
  const char *secret; // base32 encoded secret
  const char *description; // description
} OTPInfo;

totpType_t selectedOTPType;
const char *selectedOTPSecret;

OTPInfo OTP_INFO[] = {
  {
    .type = totpGoogle,
    .secret = "secretsecret",
    .description = "OTP"
  }
};

const GPathInfo REMAIN_TIME_SEGMENT_PATH_POINTS = {
  3,
  (GPoint []) {
    {0, 0},
    {-6, -60},
    {6,  -60},
  }
};
GPath remainTimeSegmentPath;

// UI

// Main Window
Window window;
SimpleMenuLayer mainMenuLayer;
SimpleMenuItem mainItems[sizeof(OTP_INFO) / sizeof(OTPInfo)];
SimpleMenuSection mainSection = 
  {
    .num_items = sizeof(OTP_INFO) / sizeof(OTPInfo),
    .title = "OTP",
    .items = (const SimpleMenuItem *)&mainItems
  };

// OTP Window
Window otpWindow;
Layer remainTimeSegmentLayer;
TextLayer OTPCodeTextLayer, timeTextLayer, timeZoneTextLayer;
ActionBarLayer timezoneSelectBar;
HeapBitmap imageSheetContainer;
GBitmap iconPrev, iconNext, iconTime;

char OTPCodeTextBuffer[10] = {0}, remainTimeTextBuffer[4] = {0}, selectedTimeZoneTextBuffer[11] = {0}, OTPCodeStringFormatBuffer[7] = {0};
int8_t remainTime = 0;
enum {
	st_mainMenuWin, st_OTPWin
} state;

// Function Prototypes
void updateOTPWindow();

void tickEventHandler(AppContextRef app_ctx, PebbleTickEvent *event) {
  updateOTPWindow();
}

void updateOTPWindow() {
  if (state != st_OTPWin) {
    return;
  }
  --remainTime;
  if (true) {
    snprintf(OTPCodeTextBuffer, 10, OTPCodeStringFormatBuffer, generateCode(selectedOTPSecret, selectedOTPType, &remainTime));
    text_layer_set_text(&OTPCodeTextLayer, OTPCodeTextBuffer);
  }
  snprintf(remainTimeTextBuffer, 4, "%d", remainTime);
  text_layer_set_text(&timeTextLayer, remainTimeTextBuffer);
}

void updateRemainTimeSegment(Layer *me, GContext* ctx) {
  unsigned int angle = remainTime * 360 / totpInfo[selectedOTPType].seconds;
  GPoint center = grect_center_point(&me->frame);
  graphics_context_set_fill_color(ctx, GColorClear);
  graphics_fill_circle(ctx, center, 55);
  graphics_context_set_fill_color(ctx, GColorBlack);
  for(; angle < 355; angle += 6) {
    gpath_rotate_to(&remainTimeSegmentPath, (TRIG_MAX_ANGLE / 360) * angle);
    gpath_draw_filled(ctx, &remainTimeSegmentPath);
  }
  graphics_context_set_fill_color(ctx, GColorClear);
  graphics_fill_circle(ctx, center, 50);
}

void updateTimezoneText(void) {
  float timezone_val = TIMEZONE_MAP[TIMEZONE];
  bool minus = timezone_val < 0;
  if (minus) {
    timezone_val = -timezone_val;
  }
  uint8_t hour, min;
  hour = (uint8_t)timezone_val;
  min = (uint8_t)((timezone_val - hour) * 60);
  snprintf(selectedTimeZoneTextBuffer, 11, "UTC %c%02d:%02d", (minus?'-':'+'), hour, min);
  text_layer_set_text(&timeZoneTextLayer, selectedTimeZoneTextBuffer);
}

void menuSelected(int index, void *context) {
  window_stack_push(&otpWindow, true);
  selectedOTPType = OTP_INFO[index].type;
  selectedOTPSecret = OTP_INFO[index].secret;
  state = st_OTPWin;
  remainTime = 0;
  snprintf(OTPCodeStringFormatBuffer, 7, "%%0%dlu", totpInfo[selectedOTPType].digit);
  updateTimezoneText();
  updateOTPWindow(NULL, NULL);
}

void nextTimeZoneClicked(ClickRecognizerRef recognizer, void *context) {
  if (TIMEZONE == 39) {
    TIMEZONE = 0;
  } else {
    TIMEZONE += 1;
  }
  remainTime = 0;
  updateTimezoneText();
}

void prevTimeZoneClicked(ClickRecognizerRef recognizer, void *context) {
  if (TIMEZONE == 0) {
    TIMEZONE = 39;
  } else {
    TIMEZONE -= 1;
  }
  remainTime = 0;
  updateTimezoneText();
}

void click_config_provider(ClickConfig **config, void *context) {
  config[BUTTON_ID_DOWN]->click.handler = &nextTimeZoneClicked;
  config[BUTTON_ID_UP]->click.handler = &prevTimeZoneClicked;
}

void initOTPMenu() {
  for(uint8_t count = 0; count < mainSection.num_items; ++count) {
    SimpleMenuItem *item = (SimpleMenuItem *)&mainSection.items[count];
    item->callback = &menuSelected;
    item->icon = NULL;
    item->title = OTP_INFO[count].description;
    if (OTP_INFO[count].type == totpBattlenet) {
      item->subtitle = "Battle.net";
    } else if (OTP_INFO[count].type == totpGoogle) {
      item->subtitle = "Google OTP";
    }
  }
  simple_menu_layer_init(&mainMenuLayer, window.layer.frame, &window, &mainSection, 1, NULL);
}

void initMainWindow() {
  window_init(&window, "Window Name");
  window_stack_push(&window, true /* Animated */);
  initOTPMenu();
  layer_add_child(&window.layer, simple_menu_layer_get_layer(&mainMenuLayer));
}

void initOTPViewWindow() {
  window_init(&otpWindow, "OTP View");
  layer_init(&remainTimeSegmentLayer, GRect(0, 0, 124, 153));
  remainTimeSegmentLayer.update_proc = &updateRemainTimeSegment;
  layer_add_child(&otpWindow.layer, &remainTimeSegmentLayer);

  gpath_init(&remainTimeSegmentPath, &REMAIN_TIME_SEGMENT_PATH_POINTS);
  gpath_move_to(&remainTimeSegmentPath, grect_center_point(&remainTimeSegmentLayer.frame));

  text_layer_init(&timeZoneTextLayer, GRect(22, 46, 80, 18));
  text_layer_set_text_alignment(&timeZoneTextLayer, GTextAlignmentCenter);
  text_layer_set_font(&timeZoneTextLayer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  layer_add_child(&otpWindow.layer, &timeZoneTextLayer.layer);

  text_layer_init(&OTPCodeTextLayer, GRect(17, 65, 90, 24));
  text_layer_set_text_alignment(&OTPCodeTextLayer, GTextAlignmentCenter);
  text_layer_set_font(&OTPCodeTextLayer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(&otpWindow.layer, &OTPCodeTextLayer.layer);

  text_layer_init(&timeTextLayer, GRect(47, 95, 30, 18));
  text_layer_set_text_alignment(&timeTextLayer, GTextAlignmentCenter);
  text_layer_set_font(&timeTextLayer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  layer_add_child(&otpWindow.layer, &timeTextLayer.layer);
  
  action_bar_layer_init(&timezoneSelectBar);
  action_bar_layer_add_to_window(&timezoneSelectBar, &otpWindow);
  action_bar_layer_set_click_config_provider(&timezoneSelectBar,
                                             click_config_provider);
  heap_bitmap_init(&imageSheetContainer, RESOURCE_ID_BUTTONS);
  gbitmap_init_as_sub_bitmap(&iconNext, &imageSheetContainer.bmp, GRect(0, 0, 18, 18));
  gbitmap_init_as_sub_bitmap(&iconPrev, &imageSheetContainer.bmp, GRect(18, 0, 18, 18));
  gbitmap_init_as_sub_bitmap(&iconTime, &imageSheetContainer.bmp, GRect(36, 0, 18, 18));
  action_bar_layer_set_icon(&timezoneSelectBar, BUTTON_ID_UP, &iconPrev);
  action_bar_layer_set_icon(&timezoneSelectBar, BUTTON_ID_DOWN, &iconNext);
  action_bar_layer_set_icon(&timezoneSelectBar, BUTTON_ID_SELECT, &iconTime);
}

void appInitHandler(AppContextRef ctx) {
  (void)ctx;
  resource_init_current_app(&APP_RESOURCES);

  // Main Window - OTP Lists
  initMainWindow();

  // OTP View Window
  initOTPViewWindow();
}

void appDeinitHandler(AppContextRef ctx) {
  heap_bitmap_deinit(&imageSheetContainer);
}


void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &appInitHandler,
    .deinit_handler = &appDeinitHandler,
    .tick_info = {
      .tick_handler = &tickEventHandler,    // called repeatedly, each second
      .tick_units = SECOND_UNIT        // specifies interval of `tick_handler`
    }
  };
  app_event_loop(params, &handlers);
}
