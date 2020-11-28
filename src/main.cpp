#include <Arduino.h>
#include <FastLED.h>
#include <Keypad.h>
#include <LiquidCrystal.h>
#include <Wire.h>

#define LED_PIN 53
#define COLOR_ORDER GRB
#define CHIPSET WS2812B
#define NUM_LEDS 50
#define FRAMES_PER_SECOND 30
#define BASE_COLOR CHSV(HUE, SAT, BRIGHTNESS_FLOOR);
#define ROWS 4
#define COLS 4

//LED stuff
CRGB leds[NUM_LEDS];
int BRIGHTNESS_CEILING = 180;
int BRIGHTNESS_FLOOR = 20;
int HUE = 31;
int SAT = 195;

//Keypad stuff
char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

//LCD stuff
const int rs = 34, en = 32, d4 = 28, d5 = 26, d6 = 24, d7 = 22;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

enum PixelState
{
  Low,
  Brightening,
  Dimming,
  High
};

struct PixelInfo
{
  PixelState state;
  int twinkleness;
  int brightness;
  int twinkleRate;
};

PixelInfo pi[NUM_LEDS];

void initLeds()
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = BASE_COLOR;
    pi[i].state = Low;
    pi[i].twinkleness = 0;
    pi[i].brightness = BRIGHTNESS_FLOOR;
    pi[i].twinkleRate = 0;
  }
}

void brighten(int i)
{

  if (pi[i].brightness < pi[i].twinkleness)
  {
    pi[i].brightness += pi[i].twinkleRate;
    if (pi[i].brightness > pi[i].twinkleness)
      pi[i].brightness = pi[i].twinkleness;
    leds[i] = CHSV(HUE, SAT, pi[i].brightness);
  }
  else
  {
    pi[i].brightness = pi[i].twinkleness;
    leds[i] = CHSV(HUE, SAT, pi[i].brightness);
    pi[i].state = High;
  }
}

void dim(int i)
{

  if (pi[i].brightness > BRIGHTNESS_FLOOR)
  {
    pi[i].brightness -= pi[i].twinkleRate;
    if (pi[i].brightness < BRIGHTNESS_FLOOR)
      pi[i].brightness = BRIGHTNESS_FLOOR;
    leds[i] = CHSV(HUE, SAT, pi[i].brightness);
  }
  else
  {
    pi[i].brightness = BRIGHTNESS_FLOOR;
    leds[i] = CHSV(HUE, SAT, pi[i].brightness);
    pi[i].state = Low;
  }
}

void twinkle()
{
  int index = rand() % NUM_LEDS;
  if (pi[index].state == Low)
  {
    pi[index].state = Brightening;
    pi[index].twinkleness = (rand() % (BRIGHTNESS_CEILING - BRIGHTNESS_FLOOR - 15)) + BRIGHTNESS_FLOOR + 15;
    pi[index].twinkleRate = (rand() % 5) + 1;
  }
  else if (pi[index].state == High)
  {
    pi[index].state = Dimming;
  }

  for (int i = 0; i < NUM_LEDS; i++)
  {
    switch (pi[i].state)
    {
    case Low:
      pi[i].brightness = BRIGHTNESS_FLOOR;
      leds[i] = CHSV(HUE, SAT, pi[i].brightness);
      break;
    case High:
      if (pi[i].twinkleness > BRIGHTNESS_CEILING)
      {
        pi[i].twinkleness = BRIGHTNESS_CEILING;
        pi[i].brightness = BRIGHTNESS_CEILING;
      }
      leds[i] = CHSV(HUE, SAT, pi[i].brightness);
      break;
    case Brightening:
      if (pi[i].twinkleness > BRIGHTNESS_CEILING)
      {
        pi[i].twinkleness = BRIGHTNESS_CEILING;
      }
      brighten(i);
      break;
    case Dimming:
      dim(i);
      break;
    default:
      break;
    }
  }
  FastLED.show();
  FastLED.delay(1000 / FRAMES_PER_SECOND);
}

void clearRow()
{
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
}

int getNum()
{
  lcd.setCursor(0, 1);
  int num = 0;
  char key1 = keypad.getKey();
  while (key1 != '#')
  {
    switch (key1)
    {
    case NO_KEY:
      break;

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      lcd.print(key1);
      num = num * 10 + (key1 - '0');
      if (num > 255)
      {
        num = 0;
        clearRow();
      }
      break;

    case '#':
      return num;
      break;

    case '*':
      num = 0;
      clearRow();
      break;

    default:
      break;
    }
    key1 = keypad.getKey();
  }
  return num;
}

void setHue()
{
  lcd.clear();
  lcd.print("Hue:");
  HUE = getNum();
  // Serial.print("Hue set at: ");
  // Serial.println(HUE);
  lcd.clear();
}

void setSat()
{
  lcd.clear();
  lcd.print("Saturation:");
  SAT = getNum();
  // Serial.print("Saturation set at: ");
  // Serial.println(SAT);
  lcd.clear();
}

void setLowBright()
{
  lcd.clear();
  lcd.print("Brightness Low:");
  BRIGHTNESS_FLOOR = getNum();
  // Serial.print("Brightness floor set at: ");
  // Serial.println(BRIGHTNESS_FLOOR);
  lcd.clear();
}

void setHighBright()
{
  lcd.clear();
  lcd.print("Brightness High:");
  BRIGHTNESS_CEILING = getNum();
  // Serial.print("Brightness ceiling set at: ");
  // Serial.println(BRIGHTNESS_CEILING);
  lcd.clear();
}

void getInput()
{
  char key = keypad.getKey();
  if (key)
  {
    switch (key)
    {
    case 'A':
      setHue();
      break;
    case 'B':
      setSat();
      break;
    case 'C':

      setLowBright();
      break;
    case 'D':
      setHighBright();
      break;
    default:
      lcd.clear();
      lcd.print("Select A B C D");
      break;
    }
  }
}

void setup()
{
  lcd.begin(16, 2);
  Serial.begin(9600);
  delay(3000);
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  initLeds();
}

void loop()
{
  getInput();
  twinkle();
}