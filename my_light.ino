#include <FastLED.h>

#define NUM_LEDS 70
#define DATA_PIN 4

#define ENC_KEY 12
#define ENC_S1 2
#define ENC_S2 3

#include <EncButton2.h>
EncButton2<EB_BTN> enc(INPUT, 12);

// encoder reads with table
long enc_pre_pos = 0;
long enc_prev_pre_pos = enc_pre_pos;
long enc_pos = 0;                     // current encoder position
long enc_prev_pos = enc_pos;          // privious encoder position
byte lastState = 0;
bool enc_btn_pressed = 0;
const int8_t increment[16] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};

short n = 0;
bool btn_state = 0;

#define MAX_BRIGHTNESS 255
#define MIN_BRIGHTNESS 5
#define BRIGHTNESS_STEP 10
CRGB leds[NUM_LEDS];
int brightness = MAX_BRIGHTNESS;

#define COLOR_TEMPERATURE_COUNT 130
#define COLOR_TEMPERATURE_STEP 1
int current_temperature = 0;
#include "color_temperatures.h"

#define COLOR_COUNT 5
int current_color = 0;
int Colors[][3] {
    {255, 0, 0},
    {0, 255, 0},
    {0, 0, 255},
    {106, 0, 255},  // blue purple
    {255, 0, 128},  // red purple
};

void setup() {
  Serial.begin(9600);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
  set_color_temperature(ColorTemperatures[current_temperature]);
  FastLED.show();
}

void loop() {
  enc_tick();
  enc.tick();

  if (enc.press()) enc_btn_pressed = true;
  else if (enc.release()) enc_btn_pressed = false;

  if (!enc_btn_pressed) {
    if (enc_rot_right()) brightness_up();
    if (enc_rot_left()) brightness_down();
  } else {
    if (enc_rot_right()) color_temperature_prev();
    if (enc_rot_left()) color_temperature_next();
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
   // Serial.println(enc_pos); 
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
  if (current_temperature < COLOR_TEMPERATURE_COUNT) {
    current_temperature += COLOR_TEMPERATURE_STEP;
  }
  set_color_temperature(ColorTemperatures[current_temperature]);
  Serial.println();
  Serial.print("bnt1 press\t current temp=");
  // Serial.println(ColorTemperatures[current_temperature]);
}

void color_temperature_prev() {
  if (current_temperature > 0) {
    current_temperature -= COLOR_TEMPERATURE_STEP;
  }
  set_color_temperature(ColorTemperatures[current_temperature]);
  Serial.println();
  Serial.print("bnt1 press\t current temp=");
  // Serial.println(ColorTemperatures[current_temperature]);
}

// Brightness
void brightness_up() {
  if (brightness < MAX_BRIGHTNESS)
    if (brightness + BRIGHTNESS_STEP <= MAX_BRIGHTNESS) {
      brightness += BRIGHTNESS_STEP;
    } else {
      brightness = MAX_BRIGHTNESS;
    }
  FastLED.setBrightness(brightness);
  
  // Serial.println();
  Serial.print("set brightness=");
  Serial.println(brightness);
}

void brightness_down() {
  if (brightness > MIN_BRIGHTNESS)
    if (brightness - BRIGHTNESS_STEP >= MIN_BRIGHTNESS) {
        brightness -= BRIGHTNESS_STEP;
    } else {
      brightness = MIN_BRIGHTNESS;
    }
  FastLED.setBrightness(brightness);

  // Serial.println();
  Serial.print("set brightness=");
  Serial.println(brightness);
}

// Colors
void color_next() {
  if (current_color < COLOR_COUNT) {
    current_color++;
  } else if (current_color == COLOR_COUNT) {
    current_color = 0;
  }
  fill_led_strip(Colors[current_color]);
}

void color_prev() {
  if (current_color > 0) {
    current_color--;
  } else if (current_color == 0) {
    current_color = COLOR_COUNT;
  }
  fill_led_strip(Colors[current_color]);
}

