#include <pebble.h>

static int s_battery_level;
static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_display;
static Layer *s_battery_layer;

static void battery_update_proc(Layer *layer, GContext *ctx)
{
  GRect bounds = layer_get_bounds(layer);
  int width = (s_battery_level * 114) / 100;
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 0, GCornerNone);
}

static void battery_callback(BatteryChargeState state)
{
  s_battery_level = state.charge_percent;
  layer_mark_dirty(s_battery_layer);
}

static void update_time()
{
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);
  text_layer_set_text(s_time_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
  update_time();
}

static void main_window_load(Window *window)
{
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  s_time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));
  s_display = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(58, 10), bounds.size.w, 50));
  window_set_background_color(s_main_window, GColorVividCerulean);
  text_layer_set_background_color(s_time_layer, GColorVividCerulean);
  text_layer_set_background_color(s_display, GColorVividCerulean);
  text_layer_set_text_color(s_time_layer, GColorDarkGray);
  text_layer_set_text_color(s_display, GColorDarkGray);
  text_layer_set_text(s_display, "LEXUGE |  DEVELOPER");
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_font(s_display, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_display, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_display));
  update_time();
  
  s_battery_layer = layer_create(GRect(14, 120, 115, 2));
  layer_set_update_proc(s_battery_layer, battery_update_proc);
  layer_add_child(window_get_root_layer(window), s_battery_layer);
}

static void main_window_unload(Window *window)
{
  text_layer_destroy(s_time_layer);
  layer_destroy(s_battery_layer);
}
static void init()
{
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
  battery_state_service_subscribe(battery_callback);
  battery_callback(battery_state_service_peek());
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  update_time();
}

static void deinit()
{
  window_destroy(s_main_window);
}

int main(void)
{
  init();
  app_event_loop();
  deinit();
}
