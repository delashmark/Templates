#include <pebble.h>

Window *window;

static AppTimer *timer = NULL;

static TextLayer *header_text_layer;
static TextLayer *body_text_layer;
static TextLayer *label_text_layer;

char* outString = "N/A";
char* HEADER_TEXT = "Template";

// Key values for AppMessage Dictionary
enum {
	STATUS_KEY = 0,	
	MESSAGE_KEY = 1,
};

void update_text() {
  	static char body_text[112];
  	snprintf(body_text, sizeof(body_text), "%s ", outString);
  	text_layer_set_text(body_text_layer, body_text);
  	
  	/*
	static char label_text[8];
	if (recording) snprintf(label_text, sizeof(label_text), "%s ", "REC");
	else snprintf(label_text, sizeof(label_text), "%s ", "");
  	text_layer_set_text(label_text_layer, label_text);
	*/
}

// Write message to buffer & send
void send_message(int STATUS, char* MSG) {
	DictionaryIterator *iter;
	
	app_message_outbox_begin(&iter);
	dict_write_int32(iter, STATUS_KEY, STATUS);
	dict_write_cstring(iter, MESSAGE_KEY, MSG);
	dict_write_end(iter);
  	app_message_outbox_send();
}

static void timer_callback(void *data) {
	send_message(3, "timer");
	timer = app_timer_register(5000, timer_callback, NULL);
}

// Called when a message is received from PebbleKitJS
void in_received_handler(DictionaryIterator *received, void *context) {
	Tuple *tuple;
	
	int STATUS = 0;	
	
	tuple = dict_find(received, STATUS_KEY);
	if (tuple) {
		STATUS = (int)tuple->value->uint32;
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Received Status: %d", STATUS);
	}
	
	tuple = dict_find(received, MESSAGE_KEY);
	if (tuple) {
		outString = tuple->value->cstring;
		if (STATUS < 1) {
			HEADER_TEXT = outString;
			outString = "";
		}
		update_text();
	}
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "Received Message: %s", tuple->value->cstring);
}

// Called when an incoming message from PebbleKitJS is dropped
void in_dropped_handler(AppMessageResult reason, void *context) {	
}

// Called when PebbleKitJS does not acknowledge receipt of a message
void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
}

void up_pressed_handler(ClickRecognizerRef recognizer, void *context) {
	send_message(1, "U");
	//vibes_short_pulse();
}

void up_released_handler(ClickRecognizerRef recognizer, void *context) {
	send_message(0, "U");
}

void down_pressed_handler(ClickRecognizerRef recognizer, void *context) {
	send_message(1, "D");
	//vibes_double_pulse();
}

void down_released_handler(ClickRecognizerRef recognizer, void *context) {
	send_message(0, "D");
}

void select_pressed_handler(ClickRecognizerRef recognizer, void *context) {
	send_message(1, "S");
	//vibes_long_pulse();
}

void select_released_handler(ClickRecognizerRef recognizer, void *context) {
	send_message(0, "S");
}

void click_config_provider(void *context) {
  	//const uint16_t repeat_interval_ms = 50;
  	//window_long_click_subscribe(BUTTON_ID_UP, (ClickHandler) up_pressed_handler);
  	window_long_click_subscribe(BUTTON_ID_UP, 200, (ClickHandler) up_pressed_handler, (ClickHandler) up_released_handler);
  	window_long_click_subscribe(BUTTON_ID_DOWN, 200, (ClickHandler) down_pressed_handler, (ClickHandler) down_released_handler);
  	window_long_click_subscribe(BUTTON_ID_SELECT, 200, (ClickHandler) select_pressed_handler, (ClickHandler) select_released_handler);
  	//window_single_repeating_click_subscribe(BUTTON_ID_DOWN, repeat_interval_ms, (ClickHandler) down_click_handler);
}

void window_load(Window *me) {
	window_set_click_config_provider(window, (ClickConfigProvider) click_config_provider);
	
  	Layer *layer = window_get_root_layer(me);
  	const int16_t width = layer_get_frame(layer).size.w;// - ACTION_BAR_WIDTH - 3;
	const int16_t height = layer_get_frame(layer).size.h;// - ACTION_BAR_WIDTH - 3;
	
	header_text_layer = text_layer_create(GRect(0, -4, width, height));
  	text_layer_set_font(header_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_text_alignment(header_text_layer, GTextAlignmentCenter);
  	text_layer_set_background_color(header_text_layer, GColorClear);
  	text_layer_set_text(header_text_layer, HEADER_TEXT);
  	layer_add_child(layer, text_layer_get_layer(header_text_layer));
	
  	body_text_layer = text_layer_create(GRect(4, 22, width - 4, height));
  	text_layer_set_font(body_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  	text_layer_set_text_alignment(header_text_layer, GTextAlignmentCenter);
  	text_layer_set_background_color(body_text_layer, GColorClear);
  	layer_add_child(layer, text_layer_get_layer(body_text_layer));
	
  	update_text();
}

void window_unload(Window *window) {
  	text_layer_destroy(header_text_layer);
  	text_layer_destroy(body_text_layer);
  	text_layer_destroy(label_text_layer);
}

void init(void) {
  	window = window_create();
	window_set_fullscreen(window, true);
  	window_set_window_handlers(window, (WindowHandlers) {
    	.load = window_load,
    	.unload = window_unload,
    });
	
	// Register AppMessage events
	app_message_register_inbox_received(in_received_handler);           
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());    //Large input and output buffer sizes
	
	timer = app_timer_register(5000, timer_callback, NULL); // First timer is set to 1 second!
	window_stack_push(window, true /* Animated */);
}

void deinit(void) {
	app_message_deregister_callbacks();
	window_destroy(window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
