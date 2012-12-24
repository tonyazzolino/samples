/* 
 
 Factory firmware for HexBright FLEX 
 v2.4  Dec 6, 2012
 
 
 TonyMod - Modified by Tony Azzolino
 v1.0  Dec 11, 2012: Added Dazzle, Fast Blink, and reworked activation order
 v1.1  Dec 13, 2012: Changed charging status to fading green LED.
 v1.2  Dec 22, 2012: Tweaked green LED on charging.
 v1.3  Dec 24, 2012: Merged LMH and HML and added program mode to switch between them
 v1.4  Dec 24, 2012: Pulled improved dazzle from https://github.com/digitalmisery/HexBrightFLEX
 
 Instructions:
 From off:
 
 In LMH Mode          In HML Mode                 In both modes
 -------------        -------------               --------------
 Short tap for:       Short tap for:              Long press for:
 Low                  High                        Dazzle
 Short tap for:       Short tap for:              Short tap for:
 Medium               Medium                      Fast blinking
 Short tap for:       Short tap for:              Short tap for:
 High                 Low                         Slow blinking
 Short tap for:       Short tap for:              Short tap for:
 Off                  Off                         Off
 
 To enter program mode, press and hold the button from the off mode. The light will go
 into Dazzle mode and then it will shut off. When you release the button, the green
 LED on the back of the HexBright will indicate what mode it is in. It will fade in
 for LMH mode and it will fade out for HML mode. When the green LED indicates the mode
 you want, press and hold the button for a half second to save your selection.
 
 A long press while any mode is activated will shut the HexBright off.
 
 */

#include <math.h>
#include <Wire.h>
#include <EEPROM.h>

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
#define MODE_PROGRAM_PREVIEW    10
#define MODE_PROGRAM            11

// State
byte mode = 0;
byte hmlbyte = 0;
boolean hml = false;
unsigned long btnTime = 0;
boolean btnDown = false;
boolean fadeDown = false;
int fadeCount = 0;
int fadeWait = 0;
int fadeOffWait = 0;
boolean gledOff = false;
boolean dazzle_on = true;
long dazzle_period = 100;

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

  // Read HML mode from EEPROM
  hmlbyte = EEPROM.read(0);
  Serial.print("EEPROM byte 0 read. Contains ");
  Serial.println(hmlbyte);
  if (hmlbyte == 255 || hmlbyte == 0)
    hml = false;
  if (hmlbyte == 1)
    hml = true;
  Serial.print("Corrected HML value is ");
  Serial.println(hml);
  Serial.println("Powered up!");
}

void loop()
{
  static unsigned long lastTime, lastTempTime, lastDazzleTime;
  unsigned long time = millis();

  // Check the state of the charge controller
  int chargeState = analogRead(APIN_CHARGE);
  if (mode != MODE_PROGRAM && mode != MODE_PROGRAM_PREVIEW){
    if (chargeState < 128)  // Low - charging
    {
      // digitalWrite(DPIN_GLED, (time&0x0100)?LOW:HIGH);
      if (!gledOff)
      {
        if (!fadeDown)
        {
          if (fadeWait == 35)
          {
            analogWrite(DPIN_GLED, fadeCount++);
            if (fadeCount == 255)
            {
              fadeDown = true;
            }
            fadeWait = 0;
          }
          else
          {
            fadeWait++;
          }
        }
        else if (fadeDown)
        {
          if (fadeWait == 25)
          {
            analogWrite(DPIN_GLED, fadeCount--);
            if (fadeCount == 0)
            {
              fadeDown = false; 
              gledOff = true;
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
      else if (gledOff && fadeOffWait < 2000)
      {
        fadeOffWait++;
      }
      else if (gledOff && fadeOffWait == 2000)
      {
        fadeOffWait = 0;
        gledOff = false;
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
  }
  // Check the temperature sensor
  if (time-lastTempTime > 1000)
  {
    lastTempTime = time;
    int temperature = analogRead(APIN_TEMP);
    if (mode != MODE_PROGRAM){
      Serial.print("Temp: ");
      Serial.println(temperature);
    }
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
    if (time - lastDazzleTime > dazzle_period)
    {
      digitalWrite(DPIN_DRV_EN, dazzle_on);
      dazzle_on = !dazzle_on;
      lastDazzleTime = time;
      dazzle_period = random(25,100);
    } 
    break;
  case MODE_BLINKFAST:
    digitalWrite(DPIN_DRV_EN, (time%150)<75);
    break;
  case MODE_PROGRAM:
    if (!gledOff)
    {
      if (hml)
      {
        if (fadeWait > 25)
        {
          analogWrite(DPIN_GLED, fadeCount--);
          fadeWait = 0;
          if (fadeCount == 0)
          {
            digitalWrite(DPIN_GLED,LOW); 
            gledOff = true;
            // Serial.print("[");
            // Serial.print(fadeCount);
            // Serial.print(">");
            fadeCount = 255;
            // Serial.print(fadeCount);
            fadeOffWait = 0;
          }
        }
        else
        {
          fadeWait++;
        }
      }
      if (!hml)
      {
        if (fadeWait > 25)
        {
          // Serial.println("Waited long enough, change the LED");
          // Serial.print(">");
          analogWrite(DPIN_GLED, fadeCount++);
          // Serial.println(fadeCount);
          fadeWait = 0;
          if (fadeCount == 255)
          {
            digitalWrite(DPIN_GLED,LOW); 
            gledOff = true;
            // Serial.print("[");
            // Serial.print(fadeCount);
            // Serial.print(">");
            fadeCount = 0;
            // Serial.print(fadeCount);
            fadeOffWait = 0;

          }
        }
        else
        {
          fadeWait++;
          // Serial.print(".");
        }
      }


    }
    else if (gledOff && fadeOffWait < 2000)
    {
      fadeOffWait++;
    }
    else if (gledOff && fadeOffWait == 2000)
    {
      gledOff = false;
      // Serial.print("]");
      // Serial.println(fadeCount);
    }
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
    if (hml && btnDown && !newBtnDown && (time-btnTime)>20)
      newMode = MODE_HIGH;
    if (!hml && btnDown && !newBtnDown && (time-btnTime)>20)
      newMode = MODE_LOW;
    if (btnDown && newBtnDown && (time-btnTime)>500)
      newMode = MODE_DAZZLING_PREVIEW;
    break;
  case MODE_LOW:
    if (btnDown && !newBtnDown && (time-btnTime)>20)
      newMode = MODE_OFF;
    if (!hml && btnDown && !newBtnDown && (time-btnTime)>20)
      newMode = MODE_MED;
    if (btnDown && newBtnDown && (time-btnTime)>500)
      newMode = MODE_OFFHOLD;
    break;
  case MODE_MED:
    if (btnDown && !newBtnDown && (time-btnTime)>20)
      newMode = MODE_LOW;
    if (!hml && btnDown && !newBtnDown && (time-btnTime)>20)
      newMode = MODE_HIGH;
    if (btnDown && newBtnDown && (time-btnTime)>500)
      newMode = MODE_OFFHOLD;
    break;
  case MODE_HIGH:
    if (btnDown && !newBtnDown && (time-btnTime)>20)
      newMode = MODE_MED;
    if (!hml && btnDown && !newBtnDown && (time-btnTime)>20)
      newMode = MODE_OFF;
    if (btnDown && newBtnDown && (time-btnTime)>500)
      newMode = MODE_OFFHOLD;
    break;
  case MODE_DAZZLING_PREVIEW:
    // This mode exists just to ignore this button release.
    if (btnDown && !newBtnDown)
      newMode = MODE_DAZZLING;
    if (btnDown && newBtnDown && (time-btnTime)>2000)
      newMode = MODE_PROGRAM_PREVIEW;
    break;
  case MODE_PROGRAM_PREVIEW:
    // This mode exists just to ignore this button release.
    if (btnDown && !newBtnDown)
      newMode = MODE_PROGRAM;
    break;
  case MODE_PROGRAM:
    if (btnDown && !newBtnDown && (time-btnTime)>20)
      {hml = !hml;Serial.print("HML set to ");Serial.println(hml);}
    if (btnDown && newBtnDown && (time-btnTime)>500)
    {
      if (hml != hmlbyte){
        Serial.println("hml doesn't match eeprom state");
        Serial.print("hmlbyte = ");
        Serial.println(hmlbyte);
        Serial.print("hml = ");
        Serial.println(hml);
        hmlbyte = hml;
        Serial.print("Writing ");
        Serial.print(hmlbyte);
        Serial.println(" to EEPROM");
        EEPROM.write(0,hmlbyte);
      }
      newMode = MODE_OFFHOLD;
    }
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
    case MODE_PROGRAM_PREVIEW:
      Serial.println("Mode = Prog preview");
      digitalWrite(DPIN_DRV_EN, LOW);
      break;
    case MODE_PROGRAM:
      Serial.println("Mode = Program");
      if (hml)
      {
        fadeCount = 255; 
        fadeWait = 0; 
        analogWrite(DPIN_GLED, fadeCount);
        Serial.println("HML is true");
      }
      if (!hml)
      {
        fadeCount = 0; 
        fadeWait = 0; 
        digitalWrite(DPIN_GLED, LOW);
        Serial.println("HML is false");
      }
      gledOff = false;
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









