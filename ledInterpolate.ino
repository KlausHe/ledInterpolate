#define serialEnable true

#ifdef serialEnable  // use DPRINT() in stead of Serial.print() to enable Serial  toggeling at compiletime
#define DPRINT(...) Serial.print(__VA_ARGS__)
#define DPRINTLN(...) Serial.println(__VA_ARGS__)
#else
#define DPRINTLN(...)
#define DPRINT(...)
#endif

#include <FastLED.h>
#include <SoftwareSerial.h>
#include <Arduino.h>  // Arduino Framework
#include <SimpleTimer.h>

SoftwareSerial mySerial(10, 11);  // RX, TX
SimpleTimer timer;

#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
// pwm-pin on Uno/Nano: 3,5,6,9,10,11; these need to be a "#define" for FastLed to work! https://forum.arduino.cc/t/fastled-addleds-with-non-constant-pin-number/622162/5
#define LedPin_A 10
#define LedPin_B 5
#define LedPin_C 6
#define LedPin_D 9

#define NUM_LEDS 79  // Array for Led-Pins for each Led-Strip

typedef struct
{
  int currentMode = 1;
  int posMin = 0;
  int posMax = 2000;
  int stripCount = 1;        // numbers of LED-Strips
  CRGB ledBuffer[NUM_LEDS];  // Frame buffer for FastLED
  int brightness = 200;      // 0-255 LED brightness scale
  int powerLimit = 900;      // 900mW Power Limit
  int hueTarget = 160;       // BLUE
  int hueActual = 96;        // GREEN
  int hueIndicator = 120;    // TEAL
  CRGB colorTarget() {
    return CHSV(hueTarget, 255, 255);
  };
  CRGB colorActual() {
    return CHSV(hueActual, 255, 255);
  };
  CRGB colorIndicator() {
    return CHSV(hueIndicator, 255, 127);
  };
  int drawLoop;
  long drawInterval = 10;
  bool indicatorEnabled = true;
  int indicatorWidth = 2;
  float indicatorSpeed = 20;
} Configuration;
Configuration Config;

//-------- STATUS ---------------
typedef struct
{
  int value = 1;
  CHSV colors[5] = {
    CHSV(96, 255, 255),   // green
    CHSV(0, 255, 255),    // red
    CHSV(160, 255, 255),  // blue
    CHSV(64, 255, 255),   // yellow
    CHSV(0, 0, 255),      // white
  };
} StatusStruct;
StatusStruct Status;
//-------- POSITION ---------------
typedef struct
{
  int target;
  int actual;
  int indicator;
  int direction = 1;
} PositionStruct;
PositionStruct Position;

//-------- LOADING ---------------
typedef struct
{
  int value = 10;
} LoadingStruct;
LoadingStruct Loading;

//-------- DEMO ---------------
typedef struct
{
  int posSpeed = 2;
} DemoConfig;
DemoConfig Demo;

void setup() {
#ifdef serialEnable
  Serial.begin(115200);
  Serial.setTimeout(10);
  Serial.println("Serial start");
#endif
  randomSeed(analogRead(A0));  // used for DemoMode -- TODO: test if this should be calles inside "loop()"
  Position.actual = Config.posMin;
  Position.target = Config.posMax;
  Position.indicator = Config.posMin;
  FastLED.addLeds<LED_TYPE, LedPin_A, COLOR_ORDER>(Config.ledBuffer, NUM_LEDS);
  if (Config.stripCount > 1) {
    FastLED.addLeds<LED_TYPE, LedPin_B, COLOR_ORDER>(Config.ledBuffer, NUM_LEDS);
  }
  if (Config.stripCount > 2) {
    FastLED.addLeds<LED_TYPE, LedPin_C, COLOR_ORDER>(Config.ledBuffer, NUM_LEDS);
  }
  if (Config.stripCount > 3) {
    FastLED.addLeds<LED_TYPE, LedPin_D, COLOR_ORDER>(Config.ledBuffer, NUM_LEDS);
  }
  FastLED.setBrightness(Config.brightness);
  // FastLED.setMaxPowerInMilliWatts(Config.powerLimit);      // Set the power limit, above which brightness will be throttled
  FastLED.clear();
  LedDrawFloatPixel(Position.actual, Config.colorActual());
  LedDrawFloatPixel(Position.target, Config.colorTarget());
  FastLED.show();
  delay(200);

  // initalize timers at last in setup to aviod hickups
  Config.drawLoop = timer.setInterval(Config.drawInterval, DrawLoop);  // time in millis, function
  timer.enable(Config.drawLoop);
  // DrawLoop();
  // timer.restartTimer(Config.drawLoop);
}
typedef void (*DrawMode[])();  // ledModes[]
DrawMode drawMode = { DrawOff, DrawStatus, DrawLoading, DrawPosition, DrawInching, DrawDemo };

void loop() {
  timer.run();
  //---------------------Serial stuff here
  if (Serial.available() != 0) {   // send Data with format "xxxxxS" --> "1302T"; T:Target, A:Actual; int xxxxx: 32768 byte
    int data = Serial.parseInt();  // keep the order "parseInt -> readString"
    String identifier = Serial.readStringUntil('\n');
    DPRINT(identifier);
    DPRINT(":  ");
    DPRINTLN(data);
    //
    if (identifier == "C" && data == 0) {
      Config.currentMode = 0;
      DrawLoopOnce();
    }
    if (identifier == "S") {
      Config.currentMode = 1;
      Status.value = data;
      DrawLoopOnce();
    }
    if (identifier == "L") {
      Config.currentMode = 2;
      Loading.value = data;
      DrawLoopOnce();
    }
    if (identifier == "T" || identifier == "A") {
      Config.currentMode = 3;
      if (identifier == "T") {
        Position.target = data;
        saveDierction(Position.target, Position.actual);
        Position.indicator = Position.actual;
      }
      if (identifier == "A") {
        Position.actual = data;
      }
      DrawLoopEnable();
    }
    if (identifier == "I") {
      Config.currentMode = 4;
      Position.actual = data;
      DrawLoopOnce();
    }
    if (identifier == "D") {
      Config.currentMode = 5;
      DrawLoopEnable();
    }
    if (identifier == "C") {
      //TODO: handle Configchanges in different Function, come back here and Draw once (for Colors etc...)
      configChangeHandler(data);
      DrawLoopOnce();
    }
  }
}

void DrawLoopOnce() {
  if (timer.isEnabled(Config.drawLoop)) {
    timer.disable(Config.drawLoop);
  }
  DrawLoop();
}

void DrawLoopEnable() {
  if (!timer.isEnabled(Config.drawLoop))
    timer.enable(Config.drawLoop);
}

// this loops independet from the  main-loop (slower) and automatically draws the current drawMode
void DrawLoop() {
  FastLED.clear();
  drawMode[Config.currentMode]();
  FastLED.show();
};

void DrawOff() {
  FastLED.clear();

};  //"0C"

void DrawStatus() {
  CHSV color = Status.colors[Status.value];
  fill_solid(Config.ledBuffer, NUM_LEDS, color);
};

void DrawLoading() {
  int loadedLed = map(Loading.value, 0, 100, 0, NUM_LEDS);
  int color = map(Loading.value, 0, 100, 0, 190);  // 96
  fill_solid(Config.ledBuffer, loadedLed, CHSV(color, 255, 255));
}

void DrawPosition() {
  if (Config.indicatorEnabled && !positionReached()) {
    Position.indicator += Position.direction * Config.indicatorSpeed;
    if ((Position.direction == 1 && Position.indicator >= Position.target) || (Position.direction == -1 && Position.indicator <= Position.target)) {
      Position.indicator = Position.actual;
    };
    LedDrawFloatPixel(Position.indicator, Config.colorIndicator());
  };
  LedDrawFloatPixel(Position.actual, Config.colorActual());
  LedDrawFloatPixel(Position.target, Config.colorTarget());
};

void DrawInching() {
  LedDrawFloatPixel(Position.actual, Config.colorActual());
};

void DrawDemo() {
  if (positionReached()) {
    Position.target = random(Config.posMin, Config.posMax);
    saveDierction(Position.target, Position.actual);
    DPRINT("New random: ");
    DPRINTLN(Position.target);
    return;
  };
  Position.actual += Demo.posSpeed * Position.direction;
  DrawPosition();
}

void configChangeHandler(int data) {
  DPRINTLN(data);
}
