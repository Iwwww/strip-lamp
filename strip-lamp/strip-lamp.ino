#include <FastLED.h>
#include <EncButton2.h>

/* === SETUP === */
#define NUM_LEDS 70
#define DATA_PIN 4

#define ENC_KEY 12                        // encoder button
#define ENC_S1 2
#define ENC_S2 3

/* === SETTINGS === */
// color temperature
#define COLOR_SMALL_STEP 1
#define COLOR_BIG_STEP 5
#define STARTUP_COLOR_TEMPERATURE 25      // color temperature on startup

// brightness
#define BRIGHTNESS_SMALL_STEP 10
#define BRIGHTNESS_BIG_STEP 50
#define MAX_BRIGHTNESS 255
#define MIN_BRIGHTNESS 5

// sleep timer
#define SLEEP_MODE_FADE_DELAY 6000       // in milli seconds
#define BRIGHTNESS_FADE_OUT_STEP 1000     // in milli seconds

// effects
#define BLINK_BRIGHTNESS_LOWERING 100
#define BLINK_DELAY 500

EncButton2<EB_BTN> enc(INPUT, 12);

// encoder reads with table
long enc_pre_pos = 0;
long enc_prev_pre_pos = enc_pre_pos;
long enc_pos = 0;                         // current encoder position
long enc_prev_pos = enc_pos;              // privious encoder position
byte lastState = 0;
bool enc_btn_pressed = 0;
const int8_t increment[16] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};

short n = 0;
bool btn_state = 0;

int brightness_step = BRIGHTNESS_SMALL_STEP;
int brightness = MAX_BRIGHTNESS;
CRGB leds[NUM_LEDS];

int8_t color_step = COLOR_SMALL_STEP;
int current_temperature = STARTUP_COLOR_TEMPERATURE;
#include "color_temperatures.h"

// sleep mode
bool sleepModeOn = 0;
int fade_out_brightness = brightness;
// timer
long sleep_time = 0;
long fade_time = 0;

void setup() {
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
  set_color_temperature(ColorTemperatures[current_temperature]);
  FastLED.show();
}

void loop() {
  enc_tick();
  enc.tick();

  if (enc.click()) set_step(enc.hasClicks());

  if (enc.press()) enc_btn_pressed = true;
  else if (enc.release()) enc_btn_pressed = false;

  if (!enc_btn_pressed) {
    if (enc_rot_right()) brightness_up();
    if (enc_rot_left()) brightness_down();
  } else {
    if (enc_rot_right()) color_temperature_prev();
    if (enc_rot_left()) color_temperature_next();
  }

  // sleep timer
  if (enc.hasClicks(2) && sleepModeOn == false) {
    sleep_time = millis();
    fade_time = sleep_time;
    sleepModeOn = true;
    fade_out_brightness = brightness;
    blink();
  }
  if (sleepModeOn) {
    // check timer
    if (millis() - sleep_time >= SLEEP_MODE_FADE_DELAY)
      if (millis() - fade_time >= BRIGHTNESS_FADE_OUT_STEP) {
        if (fade_out_brightness > 0)
          FastLED.setBrightness(--fade_out_brightness);
        fade_time = millis();
      }
    // exit sleep mode
    if (enc.click() || enc_rot_right() || enc_rot_left()) {
      sleepModeOn = false;
      FastLED.setBrightness(brightness);
      set_color_temperature(ColorTemperatures[current_temperature]);
    }
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

// Color temperatur
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

void set_step(int8_t factor) {
  if (color_step == COLOR_SMALL_STEP) color_step = COLOR_BIG_STEP;
  else color_step = COLOR_SMALL_STEP;

  if (brightness_step == BRIGHTNESS_SMALL_STEP) brightness_step = BRIGHTNESS_BIG_STEP;
  else brightness_step = BRIGHTNESS_SMALL_STEP;
}

void blink() {
  if (brightness - 15 > MIN_BRIGHTNESS) {
    FastLED.setBrightness(brightness - BLINK_BRIGHTNESS_LOWERING);
    FastLED.show();
    delay(BLINK_DELAY);
    FastLED.setBrightness(brightness);
  } else {
    FastLED.setBrightness(brightness + BLINK_BRIGHTNESS_LOWERING / 2);
    FastLED.show();
    delay(BLINK_DELAY);
    FastLED.setBrightness(brightness);
  }
}
