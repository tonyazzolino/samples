/* 
 
 Factory firmware for HexBright FLEX 
 v2.4  Dec 6, 2012
 
 LMH Mod - Modified by Tony Azzolino
 v1.0  Dec 11, 2012: Added Dazzle, Fast Blink, and reworked activation order
 v1.1  Dec 13, 2012: Changed charging status to fading green led.
 v1.2  Dec 14, 2012: Made LMH Version
 
 Instructions:
 From off:
 
 Short tap for:              Long press for:
 Low                         Dazzle
 Short tap for:              Short tap for:
 Medium                      Fast blinking
 Short tap for:              Short tap for:
 High                        Slow blinking
 Short tap for:              Short tap for:
 Off                         Off
 
 A long press while any mode is activated will shut the HexBright off.
 */

#include <math.h>
#include <Wire.h>

// Settings
#define OVERTEMP                340
// Pin assignments
#define DPIN_RLED_SW            2
#define DPIN_GLED               5
#define DPIN_PWR                8
#define DPIN_DRV_MODE           9
#define DPIN_DRV_EN             10
#define APIN_TEMP               0
#define APIN_CHARGE             3
// Modes
#define MODE_OFF                0
#define MODE_LOW                1
#define MODE_MED                2
#define MODE_HIGH               3
#define MODE_BLINKING           4
#define MODE_BLINKING_PREVIEW   5
#define MODE_DAZZLING           6
#define MODE_DAZZLING_PREVIEW   7
#define MODE_BLINKFAST          8
#define MODE_OFFHOLD            9

// State
byte mode = 0;
unsigned long btnTime = 0;
boolean btnDown = false;
int fadeDown = 0;
int fadeCount = 0;
int fadeWait = 0;
int fadeOffWait = 0;
int gledOff = 0;

void setup()
{
  // We just powered on!  That means either we got plugged 
  // into USB, or the user is pressing the power button.
  pinMode(DPIN_PWR,      INPUT);
  digitalWrite(DPIN_PWR, LOW);

  // Initialize GPIO
  pinMode(DPIN_RLED_SW,  INPUT);
  pinMode(DPIN_GLED,     OUTPUT);
  pinMode(DPIN_DRV_MODE, OUTPUT);
  pinMode(DPIN_DRV_EN,   OUTPUT);
  digitalWrite(DPIN_DRV_MODE, LOW);
  digitalWrite(DPIN_DRV_EN,   LOW);

  // Initialize serial busses
  Serial.begin(9600);
  Wire.begin();

  btnTime = millis();
  btnDown = digitalRead(DPIN_RLED_SW);
  mode = MODE_OFF;

  Serial.println("Powered up!");
}

void loop()
{
  static unsigned long lastTime, lastTempTime;
  unsigned long time = millis();

  // Check the state of the charge controller
  int chargeState = analogRead(APIN_CHARGE);
  if (chargeState < 128)  // Low - charging
  {
    // digitalWrite(DPIN_GLED, (time&0x0100)?LOW:HIGH);
    if (gledOff == 0)
    {
      if (fadeDown == 0)
      {
        if (fadeWait == 20)
        {
          analogWrite(DPIN_GLED, fadeCount++);
          if (fadeCount == 255)
          {
            fadeDown = 1;
          }
          fadeWait = 0;
        }
        else
        {
          fadeWait++;
        }
      }
      else if (fadeDown == 1)
      {
        if (fadeWait == 20)
        {
          analogWrite(DPIN_GLED, fadeCount--);
          if (fadeCount == 0)
          {
            fadeDown = 0; 
            gledOff = 1;
            digitalWrite(DPIN_GLED, LOW);
          }
          fadeWait = 0;
        }
        else
        {
          fadeWait++;
        }
      }
    } 
    else if (gledOff == 1 && fadeOffWait < 4000)
    {
      fadeOffWait++;
    }
    else if (gledOff == 1 && fadeOffWait == 4000)
    {
      fadeOffWait = 0;
      gledOff = 0;
    }

  }
  else if (chargeState > 768) // High - charged
  {
    digitalWrite(DPIN_GLED, HIGH);
  }
  else // Hi-Z - shutdown
  {
    digitalWrite(DPIN_GLED, LOW);    
  }

  // Check the temperature sensor
  if (time-lastTempTime > 1000)
  {
    lastTempTime = time;
    int temperature = analogRead(APIN_TEMP);
    Serial.print("Temp: ");
    Serial.println(temperature);
    if (temperature > OVERTEMP && mode != MODE_OFF)
    {
      Serial.println("Overheating!");

      for (int i = 0; i < 6; i++)
      {
        digitalWrite(DPIN_DRV_MODE, LOW);
        delay(100);
        digitalWrite(DPIN_DRV_MODE, HIGH);
        delay(100);
      }
      digitalWrite(DPIN_DRV_MODE, LOW);

      mode = MODE_LOW;
    }
  }

  // Do whatever this mode does
  switch (mode)
  {
  case MODE_BLINKING:
  case MODE_BLINKING_PREVIEW:
    digitalWrite(DPIN_DRV_EN, (time%300)<75);
    break;
  case MODE_DAZZLING:
  case MODE_DAZZLING_PREVIEW:
    if (time-lastTime < 10) break;
    lastTime = time;
    digitalWrite(DPIN_DRV_EN, random(4)<1);
    break;
  case MODE_BLINKFAST:
    digitalWrite(DPIN_DRV_EN, (time%150)<75);
    break;
  }

  // Periodically pull down the button's pin, since
  // in certain hardware revisions it can float.
  pinMode(DPIN_RLED_SW, OUTPUT);
  pinMode(DPIN_RLED_SW, INPUT);

  // Check for mode changes
  byte newMode = mode;
  byte newBtnDown = digitalRead(DPIN_RLED_SW);
  switch (mode)
  {
  case MODE_OFF:
    if (btnDown && !newBtnDown && (time-btnTime)>20)
      newMode = MODE_LOW;
    if (btnDown && newBtnDown && (time-btnTime)>500)
      newMode = MODE_DAZZLING_PREVIEW;
    break;
  case MODE_LOW:
    if (btnDown && !newBtnDown && (time-btnTime)>20)
      newMode = MODE_MED;
    if (btnDown && newBtnDown && (time-btnTime)>500)
      newMode = MODE_OFFHOLD;
    break;
  case MODE_MED:
    if (btnDown && !newBtnDown && (time-btnTime)>20)
      newMode = MODE_HIGH;
    if (btnDown && newBtnDown && (time-btnTime)>500)
      newMode = MODE_OFFHOLD;
    break;
  case MODE_HIGH:
    if (btnDown && !newBtnDown && (time-btnTime)>20)
      newMode = MODE_OFF;
    if (btnDown && newBtnDown && (time-btnTime)>500)
      newMode = MODE_OFFHOLD;
    break;
  case MODE_DAZZLING_PREVIEW:
    // This mode exists just to ignore this button release.
    if (btnDown && !newBtnDown)
      newMode = MODE_DAZZLING;
    break;
  case MODE_OFFHOLD:
    // This mode exists just to ignore this button release.
    if (btnDown && !newBtnDown)
      newMode = MODE_OFF;
    break;
  case MODE_BLINKING:
    if (btnDown && !newBtnDown && (time-btnTime)>20)
      newMode = MODE_OFF;
    if (btnDown && newBtnDown && (time-btnTime)>500)
      newMode = MODE_OFFHOLD;
    break;
  case MODE_BLINKFAST:
    if (btnDown && !newBtnDown && (time-btnTime)>20)
      newMode = MODE_BLINKING;
    if (btnDown && newBtnDown && (time-btnTime)>500)
      newMode = MODE_OFFHOLD;
    break;
  case MODE_DAZZLING:
    if (btnDown && !newBtnDown && (time-btnTime)>20)
      newMode = MODE_BLINKFAST;
    if (btnDown && newBtnDown && (time-btnTime)>500)
      newMode = MODE_OFFHOLD;
    break;
  }

  // Do the mode transitions
  if (newMode != mode)
  {
    switch (newMode)
    {
    case MODE_OFF:
    case MODE_OFFHOLD:
      Serial.println("Mode = off");
      pinMode(DPIN_PWR, OUTPUT);
      digitalWrite(DPIN_PWR, LOW);
      digitalWrite(DPIN_DRV_MODE, LOW);
      digitalWrite(DPIN_DRV_EN, LOW);
      break;
    case MODE_LOW:
      Serial.println("Mode = low");
      pinMode(DPIN_PWR, OUTPUT);
      digitalWrite(DPIN_PWR, HIGH);
      digitalWrite(DPIN_DRV_MODE, LOW);
      analogWrite(DPIN_DRV_EN, 64);
      break;
    case MODE_MED:
      Serial.println("Mode = medium");
      pinMode(DPIN_PWR, OUTPUT);
      digitalWrite(DPIN_PWR, HIGH);
      digitalWrite(DPIN_DRV_MODE, LOW);
      analogWrite(DPIN_DRV_EN, 255);
      break;
    case MODE_HIGH:
      Serial.println("Mode = high");
      pinMode(DPIN_PWR, OUTPUT);
      digitalWrite(DPIN_PWR, HIGH);
      digitalWrite(DPIN_DRV_MODE, HIGH);
      analogWrite(DPIN_DRV_EN, 255);
      break;
    case MODE_BLINKING:
    case MODE_BLINKING_PREVIEW:
      Serial.println("Mode = blinking");
      pinMode(DPIN_PWR, OUTPUT);
      digitalWrite(DPIN_PWR, HIGH);
      digitalWrite(DPIN_DRV_MODE, HIGH);
      break;
    case MODE_DAZZLING:
    case MODE_DAZZLING_PREVIEW:
      Serial.println("Mode = dazzling");
      pinMode(DPIN_PWR, OUTPUT);
      digitalWrite(DPIN_PWR, HIGH);
      digitalWrite(DPIN_DRV_MODE, HIGH);
      break;
    case MODE_BLINKFAST:
      Serial.println("Mode = fast blinking");
      pinMode(DPIN_PWR, OUTPUT);
      digitalWrite(DPIN_PWR, HIGH);
      digitalWrite(DPIN_DRV_MODE, HIGH);
      break;
    }

    mode = newMode;
  }

  // Remember button state so we can detect transitions
  if (newBtnDown != btnDown)
  {
    btnTime = time;
    btnDown = newBtnDown;
    delay(50);
  }
}


