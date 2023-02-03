#include <FastLED.h>

#define NUM_LEDS 70
#define DATA_PIN 4

#define ENC_KEY 12
#define ENC_S1 2
#define ENC_S2 3

#include <EncButton2.h>
EncButton2<EB_BTN> enc(INPUT, 12);
// EncButton2<EB_BTN> btn1(INPUT, 9);
// EncButton2<EB_BTN> btn2(INPUT, 10);

// encoder reads with table
long enc_pre_pos = 0;
long enc_prev_pre_pos = enc_pre_pos;
long enc_pos = 0;
long enc_prev_pos = enc_pos;
byte lastState = 0;
const int8_t increment[16] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};

short n = 0;
bool btn_state = 0;

#define MAX_BRIGHTNESS 255
#define MIN_BRIGHTNESS 5
#define BRIGHTNESS_STEP 10
CRGB leds[NUM_LEDS];
int brightness = MAX_BRIGHTNESS;

#define COLOR_TEMPERATURE_COUNT 8
int current_temperature = 0;
int ColorTemperatures[] {
    Candle,
    Tungsten40W,
    Tungsten100W,
    Halogen,
    CarbonArc,
    HighNoonSun,
    DirectSunlight,
    OvercastSky,
    ClearBlueSky,
};

#define COLOR_COUNT 5
int current_color = 0;
int Colors[][3] {
    {255, 0, 0},
    {0, 255, 0},
    {0, 0, 255},
    {106, 0, 255},  // blue purple
    {255, 0, 128},  // red purple
};

// modes
enum Mode {
  BRIGHTNESS_MODE,
  COLOR_TEMPERATURE_MODE,
  COLORS_MODE,
};

int mode = Mode::BRIGHTNESS_MODE;

void setup() {
  Serial.begin(9600);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
  set_color_temperature(Candle);
  FastLED.show();
}

void loop() {
  enc_tick();
  enc.tick();
  // btn1.tick();
  // btn2.tick();

  if (enc.)

  // change mode on encoder click
  if (enc.hasClicks(1)) {
    if (mode == Mode::BRIGHTNESS_MODE) {
      mode = Mode::COLOR_TEMPERATURE_MODE;
      set_color_temperature(ColorTemperatures[current_temperature]);
      Serial.println("COLOR_TEMPERATURE_MODE");
      
    } else if (mode == Mode::COLOR_TEMPERATURE_MODE) {
      mode = Mode::COLORS_MODE;
      fill_led_strip(Colors[current_color]);
      FastLED.show();
      Serial.println("COLORS_MODE");

    } else if (mode == Mode::COLORS_MODE) {
      mode = Mode::BRIGHTNESS_MODE;
      Serial.println("BRIGHTNESS_MODE");
    }
  }

  if (enc.hasClicks(2)) {
    if (mode != Mode::BRIGHTNESS_MODE) {
      mode = Mode::BRIGHTNESS_MODE;
      Serial.println("BRIGHTNESS_MODE");
    } else {
      mode = Mode::COLORS_MODE;
      fill_led_strip(Colors[current_color]);
      FastLED.show();
      Serial.println("COLORS_MODE");
    }
  }

  switch(mode) {
    case Mode::BRIGHTNESS_MODE:
      if (enc_rot_right()) {
        brightness_up();
      }

      if (enc_rot_left()) {
        brightness_down();
      }
      break;

    case Mode::COLOR_TEMPERATURE_MODE:
       if (enc_rot_right()) {
         color_temperature_prev();
       }

       if (enc_rot_left()) {
         color_temperature_next();
       }
      
      break;

    case Mode::COLORS_MODE:
      if (enc_rot_right()) {
        color_prev();
      }

      if (enc_rot_left()) {
        color_next();
      }

    default:
      break;
  }

  FastLED.show();
}

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

void set_color_temperature(int color) {
  FastLED.setTemperature( color ); // first temperature
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = color; // show indicator pixel
  }
}

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

void color_temperature_next() {
  if (current_temperature < COLOR_TEMPERATURE_COUNT) {
    current_temperature++;
  }
  set_color_temperature(ColorTemperatures[current_temperature]);
  Serial.println();
  Serial.print("bnt1 press\t current temp=");
  Serial.println(ColorTemperatures[current_temperature]);
}

void color_temperature_prev() {
  if (current_temperature > 0) {
    current_temperature--;
  }
  set_color_temperature(ColorTemperatures[current_temperature]);
  Serial.println();
  Serial.print("bnt1 press\t current temp=");
  Serial.println(ColorTemperatures[current_temperature]);
}

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

void enc_tick() {
  enc_prev_pos = enc_pos;
  byte state = digitalRead(ENC_S2) | (digitalRead(ENC_S1) << 1);
  if (state != lastState) {
    enc_pre_pos += increment[state | (lastState << 2)];
    lastState = state;
//    Serial.println(enc_pos);
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
