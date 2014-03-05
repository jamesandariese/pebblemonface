#include <pebble.h>
#include <pebble_fonts.h>

Window* window;

#define HOUR_OFFSET 0
#define MINUTE_OFFSET 3
#define SECOND_OFFSET 6

#define TEXT_BUFFER_SIZE 32

static void ping_app_message(void);

char time_buffer[TEXT_BUFFER_SIZE + 1];
char weekday_buffer[TEXT_BUFFER_SIZE + 1];
char date_buffer[TEXT_BUFFER_SIZE + 1];
char status_buffer[TEXT_BUFFER_SIZE + 1];

TextLayer *marquee_layer, *time_layer, *date_layer, *weekday_layer;
TextLayer *status_layer, *battery_layer;

static TextLayer* init_text_layer(GRect location, GColor colour, GColor background, const char *res_id, GTextAlignment alignment)
{
  TextLayer *layer = text_layer_create(location);
  text_layer_set_text_color(layer, colour);
  text_layer_set_background_color(layer, background);
  text_layer_set_text(layer, "derp");
  text_layer_set_font(layer, fonts_get_system_font(res_id));
  text_layer_set_text_alignment(layer, alignment);
 
  return layer;
}

void tick_callback(struct tm *tick_time, TimeUnits units_changed)
{ 
  /*switch (units_changed) {
  case SECOND_UNIT:
  strftime(time_buffer + */
  if (units_changed == MINUTE_UNIT) {
    ping_app_message();
  }
  
  strftime(date_buffer, TEXT_BUFFER_SIZE, "%B", tick_time);
  int dpos = strlen(date_buffer);
  if (dpos < TEXT_BUFFER_SIZE) {
    strftime(date_buffer + dpos, TEXT_BUFFER_SIZE - dpos, " %d", tick_time);
    if (date_buffer[dpos+1] == '0') {
      date_buffer[dpos+1] = date_buffer[dpos+2];
      date_buffer[dpos+2] = '\0';
    }
  }
  strftime(time_buffer, TEXT_BUFFER_SIZE, "%H:%M", tick_time);
  strftime(weekday_buffer, TEXT_BUFFER_SIZE, "%A", tick_time);
  app_log(APP_LOG_LEVEL_DEBUG, "pebblemonface.c", __LINE__, "Showing time: %s", time_buffer);
  text_layer_set_text(time_layer, time_buffer);
  text_layer_set_text(date_layer, date_buffer);
  text_layer_set_text(weekday_layer, weekday_buffer);
}

void window_load(Window *window) {
  app_log(APP_LOG_LEVEL_DEBUG, "pebblemonface.c", __LINE__, "window_load");
  window_set_background_color(window,GColorBlack);

  status_layer = init_text_layer(GRect(8, 8, 128, 160), GColorWhite, GColorClear, FONT_KEY_GOTHIC_14, GTextAlignmentLeft);
  battery_layer = init_text_layer(GRect(8, 8, 128, 160), GColorWhite, GColorClear, FONT_KEY_GOTHIC_14, GTextAlignmentRight);
  weekday_layer = init_text_layer(GRect(8, 52, 128, 160), GColorWhite, GColorClear, FONT_KEY_ROBOTO_CONDENSED_21, GTextAlignmentLeft);
  date_layer = init_text_layer(GRect(8, 75, 128, 160), GColorWhite, GColorClear, FONT_KEY_ROBOTO_CONDENSED_21, GTextAlignmentLeft);
  

  time_layer = init_text_layer(GRect(0, 100, 144, 168), GColorWhite, GColorClear, FONT_KEY_ROBOTO_BOLD_SUBSET_49, GTextAlignmentCenter);
  
  tick_timer_service_subscribe(MINUTE_UNIT, (TickHandler) tick_callback);
  
  //Get a time structure so that the face doesn't start blank
  struct tm *t;
  time_t temp;
  temp = time(NULL);
  t = localtime(&temp);
  
  //Manually call the tick handler when the window is loading
  tick_callback(t, MINUTE_UNIT);
  

  Layer *window_layer = window_get_root_layer(window);
  
  layer_add_child(window_layer, (Layer *)time_layer);
  layer_add_child(window_layer, (Layer *)date_layer);
  layer_add_child(window_layer, (Layer *)weekday_layer);
  layer_add_child(window_layer, (Layer *)status_layer);
  layer_add_child(window_layer, (Layer *)battery_layer);
  
}
  
void window_unload(Window *window) {
  text_layer_destroy(time_layer);
}

#define CONNECTED_NO 0
#define CONNECTED_YES 1
#define CONNECTED_UNKNOWN 2

static char appmessage_open_p = 0;
static char connected = CONNECTED_UNKNOWN;

void update_status() {
  if (connected == CONNECTED_YES) {
    text_layer_set_text(status_layer, "connected");
  } else if (connected == CONNECTED_NO) {
    text_layer_set_text(status_layer, "disconnected");
  } else {
    text_layer_set_text(status_layer, "unknown");
  }
}

static void set_connected() {
  if (connected != CONNECTED_YES) {
    connected = CONNECTED_YES;
    update_status();
  }
}

static void set_disconnected() {
  if (connected != CONNECTED_NO) {
    connected = CONNECTED_NO;
    update_status();
  }
}

static void set_connection_unknown() {
  if (connected != CONNECTED_UNKNOWN) {
    connected = CONNECTED_UNKNOWN;
    update_status();
  }
}

static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100%";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "+++");
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
  }
  text_layer_set_text(battery_layer, battery_text);
}


static void outbox_sent_handler(DictionaryIterator *failed, void *context) {
  set_connected();
}

static void outbox_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  set_disconnected();
}

static void ping_app_message() {
  DictionaryIterator *iterator;
  if (app_message_outbox_begin(&iterator) != APP_MSG_OK) {
    set_disconnected();
    return;
  }
  
  if (dict_write_cstring(iterator, 1, "ping") != DICT_OK) {
    set_connection_unknown();
    return;
  }
  if (app_message_outbox_send() != APP_MSG_OK) {
    set_disconnected();
    return;
  }
}

void app_message_init() {
  app_message_register_outbox_sent(outbox_sent_handler);
  app_message_register_outbox_failed(outbox_failed_handler);
  
  AppMessageResult open_res = app_message_open(APP_MESSAGE_INBOX_SIZE_MINIMUM, APP_MESSAGE_OUTBOX_SIZE_MINIMUM);
  if (open_res != APP_MSG_OK) {
    connected = 0;
    return;
  }
  
  update_status();
  ping_app_message();
}

void init() {
  time_buffer[TEXT_BUFFER_SIZE] =
    date_buffer[TEXT_BUFFER_SIZE] =
    weekday_buffer[TEXT_BUFFER_SIZE] =
    status_buffer[TEXT_BUFFER_SIZE] = '\0';

  window = window_create();
  WindowHandlers handlers = {
    .load = window_load,
    .unload = window_unload
  };
  window_set_window_handlers(window, handlers);
  window_stack_push(window, true);
  app_message_init();
  ping_app_message();

  battery_state_service_subscribe(handle_battery);
  BatteryChargeState charge_state = battery_state_service_peek();
  handle_battery(charge_state);

  app_log(APP_LOG_LEVEL_DEBUG, "pebblemonface.c", __LINE__, "Done Initializing"); 
}
  
void deinit() {
  window_destroy(window);
  tick_timer_service_unsubscribe();
  app_message_deregister_callbacks();
}
  
int main(void) {
  init();
  app_event_loop();
  deinit();
}
