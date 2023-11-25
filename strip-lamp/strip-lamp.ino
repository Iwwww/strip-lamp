#include <FastLED.h>
#include <EncButton.h>
#include "color_temperatures.h"
/* === SETUP === */
#define NUM_LEDS 70
#define DATA_PIN 4

#define ENC_S1 2
#define ENC_S2 3

/* === SETTINGS === */
// color temperature
#define COLOR_STEP 1
#define STARTUP_COLOR_TEMPERATURE 30  // less = warmer

// brightness
#define BRIGHTNESS_SMALL_STEP 10
#define INITIAL_BRIGHTNESS 100
#define MAX_BRIGHTNESS 255
#define MIN_BRIGHTNESS 2

// Buttons init
Button black_btn(8);
Button btn2(7);
Button btn3(6);
Button btn4(5);
Button enc_btn(11);

// encoder reads with table
long enc_pre_pos = 0;
long enc_prev_pre_pos = enc_pre_pos;
long enc_pos = 0;             // current encoder position
long enc_prev_pos = enc_pos;  // privious encoder position
byte lastState = 0;
const int8_t increment[16] = { 0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0 };

int brightness_step = BRIGHTNESS_SMALL_STEP;
int brightness = INITIAL_BRIGHTNESS;
CRGB leds[NUM_LEDS];

int8_t color_step = COLOR_STEP;
int current_temperature = STARTUP_COLOR_TEMPERATURE;

enum MODE {
  TEMP_BRIGHTNESS,
  TEMP,
  POLICE,
  SLEEP_TIMER,
} mode = TEMP_BRIGHTNESS;

bool enc_press_flag = false;

bool sleep_timer_flag = false;

/*  police */
bool police_switcher = false;

void setup() {
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
  FastLED.setBrightness(brightness);
  set_color_temperature(ColorTemperatures[current_temperature]);
  FastLED.show();

  Serial.begin(115200);
  Serial.println("Serial is runngin");
}

void loop() {
  enc_tick();
  enc_btn.tick();
  black_btn.tick();
  btn2.tick();
  btn3.tick();
  btn4.tick();

  if (black_btn.click()) {
    Serial.println("black btn clicked");
  }

  if (btn2.click()) {
    Serial.println("btn2 btn clicked");
  }

  if (enc_btn.click()) {
    Serial.println("enc_tick btn clicked");
  }

  if (enc_btn.hasClicks(1)) {
    mode = TEMP_BRIGHTNESS;
  }

  if (enc_btn.hasClicks(2)) {
    mode = TEMP;
  }

  if (enc_btn.hasClicks(3)) {
    mode = POLICE;
  }

  if (enc_btn.hasClicks(5)) {
    mode = SLEEP_TIMER;
    // sleep_timer();
  }

  if (enc_btn.holdFor(3000)) {
    Serial.println("enc_btn holding");
    if (sleep_timer_flag) {
      Serial.println("sleep timer disable");
      sleep_timer_flag = false;
    }
    if (mode != TEMP_BRIGHTNESS) {
      mode = TEMP_BRIGHTNESS;
      FastLED.setBrightness(brightness);
      set_color_temperature(ColorTemperatures[current_temperature]);
    }
  }

  if (enc_btn.pressing()) {
    enc_press_flag = true;
  } else {
    enc_press_flag = false;
  }

  switch (mode) {
    case TEMP_BRIGHTNESS:
      if (enc_press_flag) {
        if (enc_rot_right()) {
          Serial.println("brightness up");
          color_temperature_next();
        }
        if (enc_rot_left()) {
          Serial.println("brightness down");
          color_temperature_prev();
        }
      } else {
        if (enc_rot_right()) {
          Serial.println("encoder rotation right");
          brightness_up();
          color_temperature_next();
        }
        if (enc_rot_left()) {
          Serial.println("encoder rotation left");
          brightness_down();
          color_temperature_prev();
        }
      }
      break;
    case TEMP:
      current_temperature = STARTUP_COLOR_TEMPERATURE;
      set_color_temperature(ColorTemperatures[current_temperature]);
      mode = TEMP_BRIGHTNESS;
      break;
    case POLICE:
      police();
      break;
    case SLEEP_TIMER:
      Serial.println("sleep timer");
      sleep_timer_flag = true;
      mode = TEMP_BRIGHTNESS;
      // sleep_timer();
      break;
    default:
      break;
  }

  // uint8_t select_color = (int)(((float)brightness / (float)MAX_BRIGHTNESS) / 5 * COLOR_TEMPERATURE_COUNT);
  // set_color_temperature(
  //   ColorTemperatures[select_color]);

  if (sleep_timer_flag) {
    if (brightness > 50) {
      brightness = 50;
    }
    brightness_down();
    color_temperature_prev();
    // delay(20000);
  }

  FastLED.show();
}

// Encoder
void enc_tick() {
  enc_prev_pos = enc_pos;
  byte state = digitalRead(ENC_S2) | (digitalRead(ENC_S1) << 1);
  if (state != lastState) {
    enc_pre_pos += increment[state | (lastState << 2)];
    lastState = state;
  }

  enc_prev_pre_pos = enc_pre_pos;
  if (enc_pre_pos % 4 == 0) {
    enc_pos = enc_pre_pos / 4;
  }
}

bool enc_rot_right() {
  return enc_prev_pos - enc_pos >= 1 ? true : false;
}

bool enc_rot_left() {
  return enc_pos - enc_prev_pos >= 1 ? true : false;
}

// Led functin
void fill_led_strip(int r, int g, int b) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].setRGB(r, g, b);
  }
}

void fill_led_strip(int tmp[3]) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].setRGB(tmp[0], tmp[1], tmp[2]);
  }
}

// Color temperature
void set_color_temperature(uint8_t color[3]) {
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    leds[i].setRGB(color[0], color[1], color[2]);
  }
}

void color_temperature_next() {
  if (current_temperature + color_step < COLOR_TEMPERATURE_COUNT) {
    current_temperature += color_step;
  } else if (current_temperature + color_step >= COLOR_TEMPERATURE_COUNT) {
    current_temperature = COLOR_TEMPERATURE_COUNT;
  }
  set_color_temperature(ColorTemperatures[current_temperature]);
}

void color_temperature_prev() {
  if (current_temperature - color_step > 0) {
    current_temperature -= color_step;
  } else if (current_temperature - color_step <= 0) {
    current_temperature = 0;
  }
  set_color_temperature(ColorTemperatures[current_temperature]);
}

// Brightness
void brightness_up() {
  if (brightness < MAX_BRIGHTNESS)
    if (brightness + brightness_step <= MAX_BRIGHTNESS) {
      brightness += brightness_step;
    } else {
      brightness = MAX_BRIGHTNESS;
    }
  FastLED.setBrightness(brightness);
}

void brightness_down() {
  if (brightness > MIN_BRIGHTNESS)
    if (brightness - brightness_step >= MIN_BRIGHTNESS) {
      brightness -= brightness_step;
    } else {
      brightness = MIN_BRIGHTNESS;
    }
  FastLED.setBrightness(brightness);
}

void set_color(uint8_t num_leds_start, uint8_t num_leds_end, uint8_t r, uint8_t g, uint8_t b) {
  for (uint8_t i = num_leds_start; i < num_leds_end; i++) {
    leds[i].setRGB(r, g, b);
  }
}

void blink(uint8_t num_leds_start, uint8_t num_leds_end, uint8_t color[3], uint8_t delay_color, uint8_t delay_dim, uint8_t repeat) {
  for (uint8_t i = 0; i < repeat; i++) {
    set_color(num_leds_start, num_leds_end, 0, 0, 0);
    FastLED.show();
    delay(delay_dim);
    set_color(num_leds_start, num_leds_end, color[0], color[1], color[2]);
    // Serial.println("blink");
    FastLED.show();
    delay(delay_color);
  }
  set_color(num_leds_start, num_leds_end, 0, 0, 0);
}

void police() {
  uint8_t color_red[3] = { 255, 0, 0 };
  uint8_t color_blue[3] = { 0, 0, 255 };
  uint8_t current_color[3] = { color_red[0], color_red[1], color_red[2] };

  uint8_t num_leds_start = 0;
  uint8_t num_leds_end = NUM_LEDS / 2;

  if (police_switcher) {
    current_color[0] = color_blue[0];
    current_color[1] = color_blue[1];
    current_color[2] = color_blue[2];
    num_leds_start = NUM_LEDS / 2;
    num_leds_end = NUM_LEDS;
  }

  blink(num_leds_start, num_leds_end, current_color, 100, 30, 4);
  police_switcher = !police_switcher;
}

void sleep_timer() {
  // uint8_t color_orange[3] = { 200, 150, 0 };
  // blink(0, NUM_LEDS, color_orange, 200, 500, 3);
}
