#include <pebble.h>
#include <pebble_fonts.h>

Window* window;

#define HOUR_OFFSET 0
#define MINUTE_OFFSET 3
#define SECOND_OFFSET 6

#define TEXT_BUFFER_SIZE 32

char time_buffer[TEXT_BUFFER_SIZE + 1];
char weekday_buffer[TEXT_BUFFER_SIZE + 1];
char date_buffer[TEXT_BUFFER_SIZE + 1];


TextLayer *marquee_layer, *time_layer, *date_layer, *weekday_layer;

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
  
  weekday_layer = init_text_layer(GRect(8, 52, 144, 160), GColorWhite, GColorClear, FONT_KEY_ROBOTO_CONDENSED_21, GTextAlignmentLeft);
  date_layer = init_text_layer(GRect(8, 75, 144, 160), GColorWhite, GColorClear, FONT_KEY_ROBOTO_CONDENSED_21, GTextAlignmentLeft);
  

    time_layer = init_text_layer(GRect(0, 100, 144, 168), GColorWhite, GColorClear, FONT_KEY_ROBOTO_BOLD_SUBSET_49, GTextAlignmentCenter);

    tick_timer_service_subscribe(MINUTE_UNIT, (TickHandler) tick_callback);

    //Get a time structure so that the face doesn't start blank
#if 0
    struct tm *t;
    time_t temp;
    temp = time(NULL);
    t = localtime(&temp);
 
    //Manually call the tick handler when the window is loading
    tick_callback(t, MINUTE_UNIT);
#endif

    Layer *window_layer = window_get_root_layer(window);
    
    layer_add_child(window_layer, (Layer *)time_layer);
    layer_add_child(window_layer, (Layer *)date_layer);
    layer_add_child(window_layer, (Layer *)weekday_layer);

}
  
void window_unload(Window *window) {
  text_layer_destroy(time_layer);
}
  
void init() {
  time_buffer[TEXT_BUFFER_SIZE] = date_buffer[TEXT_BUFFER_SIZE] = weekday_buffer[TEXT_BUFFER_SIZE] = '\0';

  window = window_create();
  WindowHandlers handlers = {
    .load = window_load,
    .unload = window_unload
  };
  window_set_window_handlers(window, handlers);
  window_stack_push(window, true);
  app_log(APP_LOG_LEVEL_DEBUG, "pebblemonface.c", __LINE__, "Done Initializing");
 
}
  
void deinit() {
  window_destroy(window);
  tick_timer_service_unsubscribe();
}
  
int main(void) {
  init();
  app_event_loop();
  deinit();
}
