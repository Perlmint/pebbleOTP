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

struct otpinfo_t {
  totpType_t func;
  const char *secret;
  const char *description;
};

totpType_t curOTPFunc;
const char *curSecret;

struct otpinfo_t otpinfo[] = {
  {
    .func = totpGoogle,
    .secret = "secretsecret",
    .description = "OTP"
  }
};

const GPathInfo SEGMENT_PATH_POINTS = {
  3,
  (GPoint []) {
    {0, 0},
    {-6, -60}, // 70 = radius + fudge; 7 = 70*tan(6 degrees); 6 degrees per minute;
    {6,  -60},
  }
};
GPath segment_path;

Window window, otpWindow;
SimpleMenuLayer mainMenuLayer;
SimpleMenuItem mainItems[sizeof(otpinfo) / sizeof(struct otpinfo_t)];
SimpleMenuSection mainSection = 
  {
    .num_items = sizeof(otpinfo) / sizeof(struct otpinfo_t),
    .title = "OTP",
    .items = (const SimpleMenuItem *)&mainItems
  };
Layer timeSegmentLayer;
TextLayer otpLayer, timeLayer, timezoneLayer;
ActionBarLayer action_bar;
HeapBitmap image_container;
GBitmap iconPrev, iconNext, iconTime;
char OTP_buf[10] = {0}, time_buf[4] = {0}, timezone_buf[11] = {0}, OTP_format_buf[7] = {0};
int8_t remainTime = 0;
enum {
	st_mainMenuWin, st_OTPWin
} state;

void updateOTPWindow(AppContextRef app_ctx, PebbleTickEvent *event) {
  if (state != st_OTPWin) {
    return;
  }
  --remainTime;
  if (true) {
    snprintf(OTP_buf, 10, OTP_format_buf, generateCode(curSecret, curOTPFunc, &remainTime));
    text_layer_set_text(&otpLayer, OTP_buf);
  }
  snprintf(time_buf, 4, "%d", remainTime);
  text_layer_set_text(&timeLayer, time_buf);
}

void updateRemainTime(Layer *me, GContext* ctx) {
  unsigned int angle = remainTime * 360 / totpInfo[curOTPFunc].seconds;
  GPoint center = grect_center_point(&me->frame);
  graphics_context_set_fill_color(ctx, GColorClear);
  graphics_fill_circle(ctx, center, 55);
  graphics_context_set_fill_color(ctx, GColorBlack);
  for(; angle < 355; angle += 6) {
    gpath_rotate_to(&segment_path, (TRIG_MAX_ANGLE / 360) * angle);
    gpath_draw_filled(ctx, &segment_path);
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
  snprintf(timezone_buf, 11, "UTC %c%02d:%02d", (minus?'-':'+'), hour, min);
  text_layer_set_text(&timezoneLayer, timezone_buf);
}

void menuSelected(int index, void *context) {
  window_stack_push(&otpWindow, true);
  curOTPFunc = otpinfo[index].func;
  curSecret = otpinfo[index].secret;
  state = st_OTPWin;
  remainTime = 0;
  snprintf(OTP_format_buf, 7, "%%0%dlu", totpInfo[curOTPFunc].digit);
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

void handle_init(AppContextRef ctx) {
  (void)ctx;
  resource_init_current_app(&APP_RESOURCES);

  window_init(&window, "Window Name");
  window_stack_push(&window, true /* Animated */);
  for(uint8_t count = 0; count < mainSection.num_items; ++count) {
    SimpleMenuItem *item = (SimpleMenuItem *)&mainSection.items[count];
    item->callback = &menuSelected;
    item->icon = NULL;
    item->title = otpinfo[count].description;
    if (otpinfo[count].func == totpBattlenet) {
      item->subtitle = "Battle.net";
    } else if (otpinfo[count].func == totpGoogle) {
      item->subtitle = "Google OTP";
    }
  }
  simple_menu_layer_init(&mainMenuLayer, window.layer.frame, &window, &mainSection, 1, NULL);
  layer_add_child(&window.layer, simple_menu_layer_get_layer(&mainMenuLayer));

  window_init(&otpWindow, "OTP View");
  layer_init(&timeSegmentLayer, GRect(0, 0, 124, 153));
  timeSegmentLayer.update_proc = &updateRemainTime;
  layer_add_child(&otpWindow.layer, &timeSegmentLayer);
  gpath_init(&segment_path, &SEGMENT_PATH_POINTS);
  gpath_move_to(&segment_path, grect_center_point(&timeSegmentLayer.frame));
  text_layer_init(&timezoneLayer, GRect(22, 46, 80, 18));
  text_layer_set_text_alignment(&timezoneLayer, GTextAlignmentCenter);
  text_layer_set_font(&timezoneLayer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  layer_add_child(&otpWindow.layer, &timezoneLayer.layer);
  text_layer_init(&otpLayer, GRect(17, 65, 90, 24));
  text_layer_set_text_alignment(&otpLayer, GTextAlignmentCenter);
  text_layer_set_font(&otpLayer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(&otpWindow.layer, &otpLayer.layer);
  text_layer_init(&timeLayer, GRect(47, 95, 30, 18));
  text_layer_set_text_alignment(&timeLayer, GTextAlignmentCenter);
  text_layer_set_font(&timeLayer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  layer_add_child(&otpWindow.layer, &timeLayer.layer);
  
  action_bar_layer_init(&action_bar);
  // Associate the action bar with the window:
  action_bar_layer_add_to_window(&action_bar, &otpWindow);
  // Set the click config provider:
  action_bar_layer_set_click_config_provider(&action_bar,
                                             click_config_provider);
  heap_bitmap_init(&image_container, RESOURCE_ID_BUTTONS);
  gbitmap_init_as_sub_bitmap(&iconNext, &image_container.bmp, GRect(0, 0, 18, 18));
  gbitmap_init_as_sub_bitmap(&iconPrev, &image_container.bmp, GRect(18, 0, 18, 18));
  gbitmap_init_as_sub_bitmap(&iconTime, &image_container.bmp, GRect(36, 0, 18, 18));
  action_bar_layer_set_icon(&action_bar, BUTTON_ID_UP, &iconPrev);
  action_bar_layer_set_icon(&action_bar, BUTTON_ID_DOWN, &iconNext);
  action_bar_layer_set_icon(&action_bar, BUTTON_ID_SELECT, &iconTime);
}

void handle_deinit(AppContextRef ctx) {
  heap_bitmap_deinit(&image_container);
}


void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit,
    .tick_info = {
      .tick_handler = &updateOTPWindow,    // called repeatedly, each second
      .tick_units = SECOND_UNIT        // specifies interval of `tick_handler`
    }
  };
  app_event_loop(params, &handlers);
}
