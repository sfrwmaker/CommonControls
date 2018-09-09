#include <Arduino.h>
#include <Time.h>
#include <TimeLib.h>
#include "CommonControls.h"

//------------------------------------------ backlight of the LCD display (lite version) ----------------------
void BL::init(void) {

  pinMode(led_pin, OUTPUT);
  pinMode(sensor_pin, INPUT);
  int light = analogRead(sensor_pin);
  emp = 0;
  brightness = new_brightness = default_brightness;
  checkMS = ch_step = 0;
  use_local_time = false;
  automatic = true;
  nightly_brightness = 50;
  evening = morning = 0;                            // This value will be overwritten by config
  adjust();
}

int BL::empAverage(int v) {
  long nv = v *emp_k;
  int round_v = emp_k >> 1;
  emp += (nv - emp + round_v) / emp_k;
  int r = (emp + round_v) / emp_k;
  return r;
}
  
void BL::adjust(void) {

  if (!automatic) return;
  
  uint32_t ms = millis();
  if ((ms > ch_step) && (new_brightness != brightness)) {
    if (new_brightness > brightness) ++brightness; else --brightness;
    analogWrite(led_pin, brightness);
    ch_step = ms + ch_period;
  }

  if (ms < checkMS) return;
  checkMS = ms + period;

  // Turn off the backlight at night
  if (isDark()) {
    new_brightness = nightly_brightness;
    return;
  }
  
  int light = analogRead(sensor_pin);

  light = empAverage(light);
  if (light < b_night) {
    new_brightness = nightly_brightness;
    return;
  }

  if (off_daily && (light > b_day)) {
    new_brightness = 0;
    return;
  }

  light          = constrain(light, b_night, b_day);
  new_brightness = map(light, b_night, b_day, nightly_brightness, daily_brightness);
  new_brightness = constrain(new_brightness, nightly_brightness, daily_brightness);
}

void BL::setBrightness(byte b) {
  brightness = b;
  automatic = false;
  analogWrite(led_pin, brightness);
}

void BL::turnAuto(bool a) {
  automatic = a;
  checkMS = 0;
  if (a) adjust();    
}

void BL::setLimits(uint16_t dark, uint16_t daylight, byte br_nightly, byte br_daily, bool offDaily) {
  b_night             = dark;
  b_day               = daylight;
  daily_brightness    = br_daily;
  nightly_brightness  = br_nightly;
  off_daily           = offDaily;
}

void BL::setNightPeriod(byte Evening, byte Morning) { // Time in 10-minute intervals from midnight
  if (Evening <= Morning) return;
  if (Evening > 144)    return;
  morning = Morning;
  evening = Evening;
  use_local_time = true;
}

bool BL::isDark(void) {

  if (use_local_time) {
    long now_t = hour(); now_t *= 60;
    now_t += minute();   now_t *= 60;
    now_t += second();
    long m = long(morning) * 600;
    long e = long(evening) * 600;
    return ((now_t < m) || (now_t >= e));
  }

  long light = 0;
  for (byte i = 0; i < 4; ++i) {
    light += analogRead(sensor_pin);
    delay(20);
  }
  light >>= 2;
  return (light < b_night);
}

//------------------------------------------ class BUTTON ------------------------------------------------------
void BUTTON::changeINTR(void) {                     // Interrupt function, called when the button status changed
  
  bool keyUp = digitalRead(buttonPIN);
  unsigned long now_t = millis();
  if (!keyUp) {                                     // The button has been pressed
    if ((pt == 0) || (now_t - pt > overPress)) pt = now_t; 
  } else {
    if (pt > 0) {
      if ((now_t - pt) < shortPress) mode = 1;      // short press
        else mode = 2;                              // long press
      pt = 0;
    }
  }
}

byte BUTTON::buttonCheck(void) {               // Check the button state, called each time in the main loop

  mode = 0;
  bool keyUp = digitalRead(buttonPIN);         // Read the current state of the button
  uint32_t now_t = millis();
  if (!keyUp) {                                // The button is pressed
    if ((pt == 0) || (now_t - pt > overPress)) pt = now_t;
  } else {
    if (pt == 0) return 0;
    if ((now_t - pt) < bounce) return 0;
    if ((now_t - pt) > shortPress)             // Long press
      mode = 2;
    else
      mode = 1;
    pt = 0;
  } 
  return mode;
}

bool BUTTON::buttonTick(void) {                     // When the button pressed for a while, generate periodical ticks

  bool keyUp = digitalRead(buttonPIN);              // Read the current state of the button
  uint32_t now_t = millis();
  if (!keyUp && (now_t - pt > shortPress)) {        // The button have been pressed for a while
    if (now_t - tickTime > tickTimeout) {
       tickTime = now_t;
       return (pt != 0);
    }
  } else {
    if (pt == 0) return false;
    tickTime = 0;
  } 
  return false;
}

//------------------------------------------ class ENCODER ------------------------------------------------------
bool ENCODER::write(int16_t initPos) {
  if ((initPos >= min_pos) && (initPos <= max_pos)) {
    pos = initPos;
    return true;
  }
  return false;
}

void ENCODER::reset(int16_t initPos, int16_t low, int16_t upp, byte inc, byte fast_inc, bool looped) {
  min_pos = low; max_pos = upp;
  if (!write(initPos)) initPos = min_pos;
  increment = fast_increment = inc;
  if (fast_inc > increment) fast_increment = fast_inc;
  is_looped = looped;
}

void ENCODER::changeINTR(void) {                    // Interrupt function, called when the channel A of encoder changed
  
  bool rUp = digitalRead(mPIN);
  unsigned long now_t = millis();
  if (!rUp) {                                       // The channel A has been "pressed"
    if ((pt == 0) || (now_t - pt > overPress)) {
      pt = now_t;
      channelB = digitalRead(sPIN);
    }
  } else {
    if (pt > 0) {
      byte inc = increment;
      if ((now_t - pt) < overPress) {
        if ((now_t - changed) < fast_timeout) inc = fast_increment;
        changed = now_t;
        if (channelB) pos -= inc; else pos += inc;
        if (pos > max_pos) { 
          if (is_looped)
            pos = min_pos;
          else 
            pos = max_pos;
        }
        if (pos < min_pos) {
          if (is_looped)
            pos = max_pos;
          else
            pos = min_pos;
        }
      }
      pt = 0; 
    }
  }
}
