
bool positionReached() {
  return ((Position.direction == 1 && Position.actual > Position.target) || (Position.direction == -1 && Position.actual < Position.target));
}

// --------------------------- SMOOTH LED FUNCTIONS -------------------------
void LedDrawFloatPixel(int ledStrip, int pos, CRGB color)  // https://github.com/davepl/DavesGarageLEDSeries/blob/master/LED%20Episode%2009/src/main.cpp
{
  float floatingPosition = mapPositionToFloat(ledStrip, pos);
  // Calculate the percentage of brightness for the first pixel
  float percentageFirstPixel = 1.0f - (floatingPosition - (long)(floatingPosition));  // chop of everything before the "."
  float amtountFirstPixel = min(percentageFirstPixel, Config.indicatorWidth);         // draw full brightness if Config.indicatorWidth is 1 --> this is safety only
  float remaining = min(Config.indicatorWidth, FastLED.size() - floatingPosition);
  int actualDrawPixel = floatingPosition;  // -->set first pixel to th actual position as int: 58,38 --> 58

  // Blend (add) in the color of the first partial pixel
  if (remaining > 0.0f) {
    Config.ledBuffer[ledStrip][actualDrawPixel] += ColorFraction(color, amtountFirstPixel);
    actualDrawPixel++;
    remaining -= amtountFirstPixel;
  }

  // Now draw any full pixels in the middle
  while (remaining > 1.0f) {
    Config.ledBuffer[ledStrip][actualDrawPixel] += color;
    actualDrawPixel++;
    remaining--;
  }

  // Draw tail pixel, up to a single full pixel
  if (remaining > 0.0f) {
    Config.ledBuffer[ledStrip][actualDrawPixel] += ColorFraction(color, remaining);
  }
}

// FractionalColor -  returns a fraction of a color
CRGB ColorFraction(CRGB colorInput, float fraction)  // https://github.com/davepl/DavesGarageLEDSeries/blob/master/LED%20Episode%2009/src/main.cpp
{
  fraction = min(1.0f, fraction);
  return CRGB(colorInput).fadeToBlackBy(255 * (1.0f - fraction));  // dim color by a fraction (0.25 --> 25% of its brightness)
}

float mapPositionToFloat(int ledStrip, int pos) {
  return mapFloat(pos, Position.min, Position.max, 0, Config.numberOfLeds[ledStrip] - Config.indicatorWidth);
}

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (float)((x - in_min) * (out_max - out_min) / (float)(in_max - in_min)) + out_min;
}

int sign(int _a) {
  return (_a < 0) ? -1 : 1;
}