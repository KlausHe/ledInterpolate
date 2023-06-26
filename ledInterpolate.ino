#define serialEnable false
#define indicatorEnable true
#define indicatorMapColor false
#define demoEnable true
#define demoStepEnable false

#ifdef serialEnable  // use DPRINT() in stead of Serial.print() to enable Serial  toggeling at compiletime
#define DPRINT(...) Serial.print(__VA_ARGS__)
#define DPRINTLN(...) Serial.println(__VA_ARGS__)
#else
#define DPRINTLN(...)
#define DPRINT(...)
#endif

#include <SoftwareSerial.h>
#include <Arduino.h>  // Arduino Framework
#include <SimpleTimer.h>
#include <FastLED.h>

// SoftwareSerial mySerial(10, 11);  // RX, TX
SimpleTimer timer;

#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
// pwm-pin on Uno/Nano: 3,5,6,9,10,11; these need to be a "#define" for FastLed to work! https://forum.arduino.cc/t/fastled-addleds-with-non-constant-pin-number/622162/5
#define LedPin_A 10
#define LedPin_B 11

typedef struct
{
  int stripCount = 1;                // numbers of LED-Strips
  int numberOfLeds[2] = { 79, 79 };  // Array for Led-Pins for each Led-Strip
  CRGB ledBuffer[79][2] = { 0, 0 };  // Frame buffer for FastLED
  int brightness = 200;              // 0-255 LED brightness scale
  int powerLimit = 900;              // 900mW Power Limit
  int hueTarget = 160;               // BLUE
  int hueActual = 96;                // GREEN
  int hueIndicator = 120;            // TEAL
  // CRGB colorTarget = CHSV(hueTarget, 255, 255);
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
  int indicatorWidth = 2;
  float indicatorSpeed = 10;
} Configuration;
Configuration Config;

typedef struct
{
  int target;         // Position in mm über dem Boden; wird mit Position.min verrechnet
  int actual;         // aktuelle Liftposition
  int indicator;      // Position für Laufindikator, falls aktiviert
  int min = 0;        // in mm über dem Hallenboden;
  int max = 2000;     // in mm über dem Hallenboden;
  int direction = 1;  // Laufrichtung, wird in
} Pos;
Pos Position;

#ifdef demoEnable
typedef struct
{
  int posLoop;
  int posSpeed = 2;
  long demoInterval = 10;
} DemoConfiguration;
DemoConfiguration DemoConfig;
#endif

void setup() {
#ifdef serialEnable
  Serial.begin(115200);
  Serial.setTimeout(10);
  Serial.println("Serial start");
#endif
  randomSeed(analogRead(A0));
  Position.actual = Position.min;
  Position.target = Position.max;
  Position.indicator = Position.min;
  FastLED.addLeds<LED_TYPE, LedPin_A, COLOR_ORDER>(Config.ledBuffer[0], Config.numberOfLeds[0]);
  if (Config.stripCount > 1) {  // copy this if-block for more stripst
    FastLED.addLeds<LED_TYPE, LedPin_B, COLOR_ORDER>(Config.ledBuffer[1], Config.numberOfLeds[1]);
  }
  FastLED.setBrightness(Config.brightness);
  // FastLED.setMaxPowerInMilliWatts(Config.powerLimit);      // Set the power limit, above which brightness will be throttled
  FastLED.clear();
  for (int i = 0; i < Config.stripCount; i++) {
    LedDrawFloatPixel(i, Position.actual, Config.colorActual());
    LedDrawFloatPixel(i, Position.target, Config.colorTarget());
  }
  FastLED.show();
  delay(200);

  // initalize timers at last in setup to aviod hickups
  Config.drawLoop = timer.setInterval(Config.drawInterval, DrawLoopUpdate);  // time in millis, function
#ifdef demoEnable
  DemoConfig.posLoop = timer.setInterval(DemoConfig.demoInterval, DemoPosLoopUpdate);  // time in millis, function
#endif
}

void loop() {
  timer.run();

  //---------------------Serial stuff here
  if (Serial.available() != 0) {  // send Data with format "xxxS" --> "1302T"; T:Target, A:Actual
    int newPosition = Serial.parseInt();
    String identifier = Serial.readStringUntil('\n');
    DPRINT(identifier);
    DPRINT(":  ");
    DPRINTLN(newPosition);

    if (identifier == "T") {
      Position.target = newPosition;
    } else if (identifier == "A") {
      Position.actual = newPosition;
    }
    timer.enable(Config.drawLoop);
  }
}

void DrawLoopUpdate() {
  for (int ledStrip = 0; ledStrip < Config.stripCount; ledStrip++) {
    FastLED.clear();
#ifdef indicatorEnable
    if (!positionReached()) {
      Position.indicator += Position.direction * Config.indicatorSpeed;
      if ((Position.direction == 1 && Position.indicator >= Position.target) || (Position.direction == -1 && Position.indicator <= Position.target)) {
        Position.indicator = Position.actual;
      };
#ifdef indicatorMapColor
      Config.hueIndicator = map(Position.indicator, Position.actual, Position.target, Config.hueActual, Config.hueTarget);
#endif
      // indicator first
      LedDrawFloatPixel(ledStrip, Position.indicator, Config.colorIndicator());
    };
#endif
    LedDrawFloatPixel(ledStrip, Position.actual, Config.colorActual());
    LedDrawFloatPixel(ledStrip, Position.target, Config.colorTarget());
    FastLED.show();
  }
}
