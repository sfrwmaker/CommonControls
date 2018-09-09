#ifndef CommonControls_h
#define CommonControls_h

//------------------------------------------ backlight of the LCD display (lite version) ----------------------
class BL {
  public:
    BL(byte sensorPIN, byte lightPIN, byte start_brightness = 128) {
      sensor_pin          = sensorPIN;
      led_pin             = lightPIN;
      default_brightness  = start_brightness;
      b_night             =  50;
      daily_brightness    = 150;
      b_day               = 500;
    }
    void init(void);                                // Initialize the data
    void adjust(void);                              // Automatically adjust the brightness
    int  getSensorValue(void)                       { return analogRead(sensor_pin); }
    void setBrightness(byte b);
    void turnAuto(bool a);
    void setLimits(uint16_t dark, uint16_t daylight, byte br_nightly, byte br_daily, bool offDaily = true);
    void setNightPeriod(byte Evening, byte Morning);
    bool isDark(void);                              // Whether it is night time or it is dark here
  private:
    int      empAverage(int v);                     // Exponential average value
    byte     sensor_pin;                            // Light sensor pin
    byte     led_pin;                               // Led PWM pin
    bool     off_daily;                             // Whether turn the light daily
    uint32_t checkMS;                               // Time in ms when the sensor was checked
    uint32_t ch_step;                               // The time in ms when the brightness can be adjusted
    bool     automatic;                             // Whether the backlight should be adjusted automatically
    bool     use_local_time;                        // Whether to use local time to switch off the light nightly
    byte     brightness;                            // The backlight brightness
    byte     new_brightness;                        // The baclight brightness to set up
    byte     evening, morning;                      // The time of evening and morning (in 10-minutes interval)
    long     emp;                                   // Exponential average value
    byte     default_brightness;                    // Default brightness of backlight
    uint16_t b_night;                               // light sensor value of the night
    uint16_t b_day;                                 // light sensor value of the day light
    byte     daily_brightness;                      // The maximum brightness of backlight when light between b_night and b_day
    byte     nightly_brightness;                    // The brightness to use nightly
    const byte     emp_k = 8;                       // The exponential average coefficient
    const uint16_t period  = 200;                   // The period in ms to check the photoresister
    const uint16_t ch_period = 5;                   // The period to adjust brightness
};

//------------------------------------------ class BUTTON ------------------------------------------------------
class BUTTON {
  public:
    BUTTON(byte ButtonPIN, unsigned int timeout_ms = 3000) {
      pt = tickTime = 0;
      buttonPIN = ButtonPIN;
      overPress = timeout_ms;
    }
    void init(void) { pinMode(buttonPIN, INPUT_PULLUP); }
    void setTimeout(uint16_t timeout_ms = 3000)    { overPress = timeout_ms; }
    byte intButtonStatus(void)                     { byte m = mode; mode = 0; return m; }
    void changeINTR(void);
    byte buttonCheck(void);
    bool buttonTick(void);
  private:
    volatile byte     mode;                         // The button mode: 0 - not pressed, 1 - pressed, 2 - long pressed
    uint16_t          overPress;                    // Maximum time in ms the button can be pressed
    volatile uint32_t pt;                           // Time in ms when the button was pressed (press time)
    uint32_t          tickTime;                     // The time in ms when the button Tick was set
    byte              buttonPIN;                    // The pin number connected to the button
    const uint16_t    tickTimeout = 200;            // Period of button tick, while the button is pressed 
    const uint16_t    shortPress  = 900;            // If the button was pressed less that this timeout, we assume the short button press
    const byte        bounce      = 50;             // Bouncing timeout (ms)
};

//------------------------------------------ class ENCODER ------------------------------------------------------
class ENCODER {
  public:
    ENCODER(byte aPIN, byte bPIN, int16_t initPos = 0) {
      pt = 0; mPIN = aPIN; sPIN = bPIN; pos = initPos;
      min_pos = -32767; max_pos = 32766; channelB = false; increment = 1;
      changed = 0;
      is_looped = false;
    }
    void init(void) {
      pinMode(mPIN, INPUT_PULLUP);
      pinMode(sPIN, INPUT_PULLUP);
    }
    void    set_increment(byte inc)             { increment = inc; }
    byte    get_increment(void)                 { return increment; }
    int16_t read(void)                          { return pos; }
    void    reset(int16_t initPos, int16_t low, int16_t upp, byte inc = 1, byte fast_inc = 0, bool looped = false);
    bool    write(int16_t initPos);
    void    changeINTR(void);
  private:
    int32_t           min_pos, max_pos;
    volatile uint32_t pt;                           // Time in ms when the encoder was rotaded
    volatile uint32_t changed;                      // Time in ms when the value was changed
    volatile bool     channelB;
    volatile int16_t  pos;                          // Encoder current position
    byte              mPIN, sPIN;                   // The pin numbers connected to the main channel and to the socondary channel
    bool              is_looped;                    // Whether the encoder is looped
    byte              increment;                    // The value to add or subtract for each encoder tick
    byte              fast_increment;               // The value to change encoder when in runs quickly
    const uint16_t    fast_timeout = 300;           // Time in ms to change encoder quickly
    const uint16_t    overPress = 1000;
};

#endif