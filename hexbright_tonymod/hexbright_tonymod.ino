/* 
 
 Factory firmware for HexBright FLEX 
 v2.4  Dec 6, 2012
 
 
 TonyMod - Modified by Tony Azzolino
 v1.0  Dec 11, 2012: Added Dazzle, Fast Blink, and reworked activation order
 v1.1  Dec 13, 2012: Changed charging status to fading green LED.
 v1.2  Dec 22, 2012: Tweaked green LED on charging.
 v1.3  Dec 24, 2012: Merged LMH and HML and added program mode to switch between them
 v1.4  Dec 24, 2012: Pulled improved dazzle from https://github.com/digitalmisery/HexBrightFLEX
 v1.5  Dec 26, 2012: Enabled HexBright to skip modes and to save modes skipped to EEPROM.
 
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
 into Dazzle mode and then it will shut off after 4 seconds. When you release the button, the green
 LED on the back of the HexBright will either fade up for LMH mode or fade down for HML mode.
 When the green LED indicates the mode you want, press and hold the button for a half second to
 go to mode enable selection. In mode enable selection, the HexBright will first go to low or high,
 depending on the LMH/HML setting. The green LED will be lit if the mode is enabled and out if it is
 disabled. A tap of the button toggles the enabled/disabled status of the current mode, while a
 a 0.5 second press moves to the next mode. After slow blink mode, the light will save its settings
 if necessary, then turn off. 
 
 A long press while any mode is activated will shut the HexBright off. (Unless it's in
 program mode. If it's in program mode, you have to finish.)
 
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
#define MODE_OFFHOLD            1
#define MODE_PRE_LOW            2
#define MODE_LOW                3
#define MODE_PRE_MED            4
#define MODE_MED                5
#define MODE_PRE_HIGH           6
#define MODE_HIGH               7
#define MODE_PRE_DAZZLE         8
#define MODE_DAZZLE             9
#define MODE_PRE_FBL            10
#define MODE_FBL                11
#define MODE_PRE_SBL            12
#define MODE_SBL                13
#define MODE_PRE_PROGRAM        14
#define MODE_PROGRAM            15

// State
byte mode = 0;
byte hmlbyte = 0;
byte skiptomode = 255;
boolean hml = false;
boolean lowskip = false, medskip = true, highskip = false, dazskip = false, fblskip = true, sblskip = false;
boolean mprog = false;
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
<<<<<<< HEAD
  if (EEPROM.read(0) == 1) hml = true; else hml = false;
  if (EEPROM.read(1) == 1) highskip = true; else highskip = false;
  if (EEPROM.read(2) == 1) medskip = true; else medskip = false;
  if (EEPROM.read(3) == 1) lowskip = true; else lowskip = false;
  if (EEPROM.read(4) == 1) dazskip = true; else dazskip = false;
  if (EEPROM.read(5) == 1) fblskip = true; else fblskip = false;
  if (EEPROM.read(6) == 1) sblskip = true; else sblskip = false;

  Serial.print("HML: ");
  Serial.println(hml);
  
    Serial.print("highskip: ");
  Serial.println(highskip);
  
    Serial.print("medskip: ");
  Serial.println(medskip);
  
    Serial.print("lowskip: ");
  Serial.println(lowskip);
  
    Serial.print("dazskip: ");
  Serial.println(dazskip);
  
    Serial.print("fblskip: ");
  Serial.println(fblskip);
  
    Serial.print("sblskip: ");
  Serial.println(sblskip);
  
=======
  if (EEPROM.read(0) == 1) hml = true; 
  else hml = false;
  if (EEPROM.read(1) == 1) highskip = true; 
  else highskip = false;
  if (EEPROM.read(2) == 1) medskip = true; 
  else medskip = false;
  if (EEPROM.read(3) == 1) lowskip = true; 
  else lowskip = false;
  if (EEPROM.read(4) == 1) dazskip = true; 
  else dazskip = false;
  if (EEPROM.read(5) == 1) fblskip = true; 
  else fblskip = false;
  if (EEPROM.read(6) == 1) sblskip = true; 
  else sblskip = false;

  Serial.print("HML: ");
  Serial.println(hml);

  Serial.print("highskip: ");
  Serial.println(highskip);

  Serial.print("medskip: ");
  Serial.println(medskip);

  Serial.print("lowskip: ");
  Serial.println(lowskip);

  Serial.print("dazskip: ");
  Serial.println(dazskip);

  Serial.print("fblskip: ");
  Serial.println(fblskip);

  Serial.print("sblskip: ");
  Serial.println(sblskip);

>>>>>>> rolled back because of bugs
  Serial.println("Powered up!");
}

void loop()
{
  static unsigned long lastTime, lastTempTime, lastDazzleTime;
  unsigned long time = millis();

  // Check the state of the charge controller
  int chargeState = analogRead(APIN_CHARGE);
  if (mode != MODE_PROGRAM && mode != MODE_PRE_PROGRAM && !mprog){
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
  case MODE_LOW:
<<<<<<< HEAD
  if(mprog)digitalWrite(DPIN_GLED, !lowskip);
  break;
  case MODE_MED:
    if(mprog)digitalWrite(DPIN_GLED, !medskip);
  break;
  case MODE_HIGH:
    if(mprog)digitalWrite(DPIN_GLED, !highskip);

  break;
=======
    if(mprog)digitalWrite(DPIN_GLED, !lowskip);
    break;
  case MODE_MED:
    if(mprog)digitalWrite(DPIN_GLED, !medskip);
    break;
  case MODE_HIGH:
    if(mprog)digitalWrite(DPIN_GLED, !highskip);

    break;
>>>>>>> rolled back because of bugs
  case MODE_SBL:
  case MODE_PRE_SBL:
    if(mprog)digitalWrite(DPIN_GLED, !sblskip);

    digitalWrite(DPIN_DRV_EN, (time%300)<75);
    break;
  case MODE_DAZZLE:
  case MODE_PRE_DAZZLE:
    if(mprog)digitalWrite(DPIN_GLED, !dazskip);

    if (time - lastDazzleTime > dazzle_period)
    {
      digitalWrite(DPIN_DRV_EN, dazzle_on);
      dazzle_on = !dazzle_on;
      lastDazzleTime = time;
      dazzle_period = random(25,100);
    } 
    break;
  case MODE_FBL:
  case MODE_PRE_FBL:
    if(mprog)digitalWrite(DPIN_GLED, !fblskip);

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
<<<<<<< HEAD
  if (skiptomode != 255) {newMode = skiptomode; skiptomode = 255;}
=======
  if (skiptomode != 255) {
    newMode = skiptomode; 
    skiptomode = 255;
  }
>>>>>>> rolled back because of bugs
  byte newBtnDown = digitalRead(DPIN_RLED_SW);
  switch (mode)
  {
  case MODE_OFF:
    if (hml && btnDown && !newBtnDown && (time-btnTime)>20)
      newMode = MODE_HIGH;
    if (!hml && btnDown && !newBtnDown && (time-btnTime)>20)
      newMode = MODE_LOW;
    if (btnDown && newBtnDown && (time-btnTime)>500)
      newMode = MODE_PRE_DAZZLE;
    break;
<<<<<<< HEAD
      case MODE_PRE_LOW:
=======
  case MODE_PRE_LOW:
>>>>>>> rolled back because of bugs
    // This mode exists just to ignore this button release.
    if (btnDown && !newBtnDown)
      newMode = MODE_LOW;
    break;
  case MODE_LOW:
    if (!mprog && hml && btnDown && !newBtnDown && (time-btnTime)>20)
      newMode = MODE_OFF;
    if (!mprog && !hml && btnDown && !newBtnDown && (time-btnTime)>20)
      newMode = MODE_MED;
    if (!mprog && btnDown && newBtnDown && (time-btnTime)>500)
      newMode = MODE_OFFHOLD;
    if (mprog && btnDown && !newBtnDown && (time-btnTime)>20)
<<<<<<< HEAD
        lowskip = !lowskip;
=======
      lowskip = !lowskip;
>>>>>>> rolled back because of bugs
    if (mprog && hml && btnDown && newBtnDown && (time-btnTime)>500)
      newMode = MODE_PRE_DAZZLE;
    if (mprog && !hml && btnDown && newBtnDown && (time-btnTime)>500)
      newMode = MODE_PRE_MED;
    break;
<<<<<<< HEAD
       case MODE_PRE_MED:
=======
  case MODE_PRE_MED:
>>>>>>> rolled back because of bugs
    // This mode exists just to ignore this button release.
    if (btnDown && !newBtnDown)
      newMode = MODE_MED;
    break;
  case MODE_MED:
    if (!mprog && hml && btnDown && !newBtnDown && (time-btnTime)>20)
      newMode = MODE_LOW;
    if (!mprog && !hml && btnDown && !newBtnDown && (time-btnTime)>20)
      newMode = MODE_HIGH;
    if (!mprog && btnDown && newBtnDown && (time-btnTime)>500)
      newMode = MODE_OFFHOLD;
    if (mprog && btnDown && !newBtnDown && (time-btnTime)>20)
<<<<<<< HEAD
        medskip = !medskip;
=======
      medskip = !medskip;
>>>>>>> rolled back because of bugs
    if (mprog && hml && btnDown && newBtnDown && (time-btnTime)>500)
      newMode = MODE_PRE_LOW;
    if (mprog && !hml && btnDown && newBtnDown && (time-btnTime)>500)
      newMode = MODE_PRE_HIGH;
    break;
<<<<<<< HEAD
      case MODE_PRE_HIGH:
=======
  case MODE_PRE_HIGH:
>>>>>>> rolled back because of bugs
    // This mode exists just to ignore this button release.
    if (btnDown && !newBtnDown)
      newMode = MODE_HIGH;
    break;
  case MODE_HIGH:
    if (!mprog && hml && btnDown && !newBtnDown && (time-btnTime)>20)
      newMode = MODE_MED;
    if (!mprog && !hml && btnDown && !newBtnDown && (time-btnTime)>20)
      newMode = MODE_OFF;
    if (!mprog && btnDown && newBtnDown && (time-btnTime)>500)
      newMode = MODE_OFFHOLD;
    if (mprog && btnDown && !newBtnDown && (time-btnTime)>20)
<<<<<<< HEAD
        highskip = !highskip;
=======
      highskip = !highskip;
>>>>>>> rolled back because of bugs
    if (mprog && hml && btnDown && newBtnDown && (time-btnTime)>500)
      newMode = MODE_PRE_MED;
    if (mprog && !hml && btnDown && newBtnDown && (time-btnTime)>500)
      newMode = MODE_PRE_DAZZLE;
    break;
  case MODE_PRE_DAZZLE:
    // This mode exists just to ignore this button release.
    if (btnDown && !newBtnDown)
      newMode = MODE_DAZZLE;
    if (!mprog && btnDown && newBtnDown && (time-btnTime)>4000)
      newMode = MODE_PRE_PROGRAM;
    break;
  case MODE_PRE_PROGRAM:
    // This mode exists just to ignore this button release.
    if (btnDown && !newBtnDown)
      newMode = MODE_PROGRAM;
    break;
  case MODE_PROGRAM:
    if (btnDown && !newBtnDown && (time-btnTime)>20)
<<<<<<< HEAD
      {hml = !hml;Serial.print("HML now ");Serial.println(hml);}
    if (btnDown && newBtnDown && (time-btnTime)>500)
    {
      if (hml) newMode = MODE_PRE_HIGH; else newMode = MODE_PRE_LOW;
=======
    {
      hml = !hml;
      Serial.print("HML now ");
      Serial.println(hml);
    }
    if (btnDown && newBtnDown && (time-btnTime)>500)
    {
      if (hml) newMode = MODE_PRE_HIGH; 
      else newMode = MODE_PRE_LOW;
>>>>>>> rolled back because of bugs
      digitalWrite(DPIN_GLED, LOW);
      mprog = true;
    }
    break;
  case MODE_OFFHOLD:
    // This mode exists just to ignore this button release.
    if (btnDown && !newBtnDown)
      newMode = MODE_OFF;
    break;
<<<<<<< HEAD
        case MODE_PRE_SBL:
=======
  case MODE_PRE_SBL:
>>>>>>> rolled back because of bugs
    // This mode exists just to ignore this button release.
    if (btnDown && !newBtnDown)
      newMode = MODE_SBL;
    break;
  case MODE_SBL:
    if (!mprog && btnDown && !newBtnDown && (time-btnTime)>20)
      newMode = MODE_OFF;
    if (!mprog && btnDown && newBtnDown && (time-btnTime)>500)
<<<<<<< HEAD
      newMode = MODE_OFFHOLD;
    if (mprog && btnDown && !newBtnDown && (time-btnTime)>20)
      sblskip = !sblskip;
    if (mprog && btnDown && newBtnDown && (time-btnTime)>500)
      {
        if (hml != EEPROM.read(0)) EEPROM.write(0, hml);
        if (highskip != EEPROM.read(1)) EEPROM.write(1, highskip);
        if (medskip != EEPROM.read(2)) EEPROM.write(2, medskip);
        if (lowskip != EEPROM.read(3)) EEPROM.write(3, lowskip);
        if (dazskip != EEPROM.read(4)) EEPROM.write(4, dazskip);
        if (fblskip != EEPROM.read(5)) EEPROM.write(5, fblskip);
        if (sblskip != EEPROM.read(6)) EEPROM.write(6, sblskip);
        gledOff=false;
        fadeCount=0;
        fadeDown=false;
        fadeWait=0;
        fadeOffWait=0;
        mprog = false;
        newMode = MODE_OFFHOLD;
      }
    break;
    
        case MODE_PRE_FBL:
=======
      newMode = MODE_OFFHOLD;
    if (mprog && btnDown && !newBtnDown && (time-btnTime)>20)
      sblskip = !sblskip;
    if (mprog && btnDown && newBtnDown && (time-btnTime)>500)
    {
      if (hml != EEPROM.read(0)) EEPROM.write(0, hml);
      if (highskip != EEPROM.read(1)) EEPROM.write(1, highskip);
      if (medskip != EEPROM.read(2)) EEPROM.write(2, medskip);
      if (lowskip != EEPROM.read(3)) EEPROM.write(3, lowskip);
      if (dazskip != EEPROM.read(4)) EEPROM.write(4, dazskip);
      if (fblskip != EEPROM.read(5)) EEPROM.write(5, fblskip);
      if (sblskip != EEPROM.read(6)) EEPROM.write(6, sblskip);
      gledOff=false;
      fadeCount=0;
      fadeDown=false;
      fadeWait=0;
      fadeOffWait=0;
      mprog = false;
      newMode = MODE_OFFHOLD;
    }
    break;

  case MODE_PRE_FBL:
>>>>>>> rolled back because of bugs
    // This mode exists just to ignore this button release.
    if (btnDown && !newBtnDown)
      newMode = MODE_FBL;
    break;
  case MODE_FBL:
    if (mprog && btnDown && !newBtnDown && (time-btnTime)>20)
      fblskip = !fblskip;
    if (mprog && btnDown && newBtnDown && (time-btnTime)>500)
      newMode = MODE_PRE_SBL;
    if (!mprog && btnDown && !newBtnDown && (time-btnTime)>20)
      newMode = MODE_SBL;
    if (!mprog && btnDown && newBtnDown && (time-btnTime)>500)
      newMode = MODE_OFFHOLD;
    break;
  case MODE_DAZZLE:
    if (mprog && btnDown && !newBtnDown && (time-btnTime)>20)
      dazskip = !dazskip;
    if (mprog && btnDown && newBtnDown && (time-btnTime)>500)
      newMode = MODE_PRE_FBL;
    if (!mprog && btnDown && !newBtnDown && (time-btnTime)>20)
      newMode = MODE_FBL;
    if (!mprog && btnDown && newBtnDown && (time-btnTime)>500)
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
    case MODE_PRE_LOW:
<<<<<<< HEAD
    if(!mprog && lowskip && hml) {Serial.println("L>O"); skiptomode = MODE_OFF;}
    else if (!mprog && lowskip && !hml) {Serial.println("L>M"); skiptomode = MODE_MED;}
    else{
      Serial.println("Mode = low");
      pinMode(DPIN_PWR, OUTPUT);
      digitalWrite(DPIN_PWR, HIGH);
      digitalWrite(DPIN_DRV_MODE, LOW);
      analogWrite(DPIN_DRV_EN, 64);}
      break;
    case MODE_MED:
    case MODE_PRE_MED:
    if(!mprog && medskip && hml) {Serial.println("M>L"); skiptomode = MODE_LOW;}
    else if (!mprog && medskip && !hml) {Serial.println("M>H"); skiptomode = MODE_HIGH;}
    else{
      Serial.println("Mode = medium");
      pinMode(DPIN_PWR, OUTPUT);
      digitalWrite(DPIN_PWR, HIGH);
      digitalWrite(DPIN_DRV_MODE, LOW);
      analogWrite(DPIN_DRV_EN, 255);}
      break;
    case MODE_HIGH:
    case MODE_PRE_HIGH:
    if(!mprog && highskip && hml) {Serial.println("H>M"); skiptomode = MODE_MED;}
    else if (!mprog && highskip && !hml) {Serial.println("H>O"); skiptomode = MODE_OFF;}
    else{
      Serial.println("Mode = high");
      pinMode(DPIN_PWR, OUTPUT);
      digitalWrite(DPIN_PWR, HIGH);
      digitalWrite(DPIN_DRV_MODE, HIGH);
      analogWrite(DPIN_DRV_EN, 255);}
      break;
    case MODE_SBL:
    case MODE_PRE_SBL:
    if(!mprog && sblskip) {Serial.println("S>O"); skiptomode = MODE_OFF;}
    else {
      Serial.println("Mode = SBl");
      pinMode(DPIN_PWR, OUTPUT);
      digitalWrite(DPIN_PWR, HIGH);
      digitalWrite(DPIN_DRV_MODE, HIGH);}
      break;
    case MODE_DAZZLE:
    if(!mprog && dazskip) {Serial.println("D>F"); skiptomode = MODE_FBL;}
    else {
      Serial.println("Mode = Daz");
      pinMode(DPIN_PWR, OUTPUT);
      digitalWrite(DPIN_PWR, HIGH);
      digitalWrite(DPIN_DRV_MODE, HIGH);}
      break;
    case MODE_FBL:
    case MODE_PRE_FBL:
        if(!mprog && fblskip) {Serial.println("F>S"); skiptomode = MODE_SBL;}
    else {
      Serial.println("Mode = FBl");
      pinMode(DPIN_PWR, OUTPUT);
      digitalWrite(DPIN_PWR, HIGH);
      digitalWrite(DPIN_DRV_MODE, HIGH);}
=======
      if(!mprog && lowskip && hml) {
        Serial.println("L>O"); 
        skiptomode = MODE_OFF;
      }
      else if (!mprog && lowskip && !hml) {
        Serial.println("L>M"); 
        skiptomode = MODE_MED;
      }
      else{
        Serial.println("Mode = low");
        pinMode(DPIN_PWR, OUTPUT);
        digitalWrite(DPIN_PWR, HIGH);
        digitalWrite(DPIN_DRV_MODE, LOW);
        analogWrite(DPIN_DRV_EN, 64);
      }
      break;
    case MODE_MED:
    case MODE_PRE_MED:
      if(!mprog && medskip && hml) {
        Serial.println("M>L"); 
        skiptomode = MODE_LOW;
      }
      else if (!mprog && medskip && !hml) {
        Serial.println("M>H"); 
        skiptomode = MODE_HIGH;
      }
      else{
        Serial.println("Mode = medium");
        pinMode(DPIN_PWR, OUTPUT);
        digitalWrite(DPIN_PWR, HIGH);
        digitalWrite(DPIN_DRV_MODE, LOW);
        analogWrite(DPIN_DRV_EN, 255);
      }
      break;
    case MODE_HIGH:
    case MODE_PRE_HIGH:
      if(!mprog && highskip && hml) {
        Serial.println("H>M"); 
        skiptomode = MODE_MED;
      }
      else if (!mprog && highskip && !hml) {
        Serial.println("H>O"); 
        skiptomode = MODE_OFF;
      }
      else{
        Serial.println("Mode = high");
        pinMode(DPIN_PWR, OUTPUT);
        digitalWrite(DPIN_PWR, HIGH);
        digitalWrite(DPIN_DRV_MODE, HIGH);
        analogWrite(DPIN_DRV_EN, 255);
      }
      break;
    case MODE_SBL:
    case MODE_PRE_SBL:
      if(!mprog && sblskip) {
        Serial.println("S>O"); 
        skiptomode = MODE_OFF;
      }
      else {
        Serial.println("Mode = SBl");
        pinMode(DPIN_PWR, OUTPUT);
        digitalWrite(DPIN_PWR, HIGH);
        digitalWrite(DPIN_DRV_MODE, HIGH);
      }
      break;
    case MODE_DAZZLE:
      if(!mprog && dazskip) {
        Serial.println("D>F"); 
        skiptomode = MODE_FBL;
      }
      else {
        Serial.println("Mode = Daz");
        pinMode(DPIN_PWR, OUTPUT);
        digitalWrite(DPIN_PWR, HIGH);
        digitalWrite(DPIN_DRV_MODE, HIGH);
      }
      break;
    case MODE_FBL:
    case MODE_PRE_FBL:
      if(!mprog && fblskip) {
        Serial.println("F>S"); 
        skiptomode = MODE_SBL;
      }
      else {
        Serial.println("Mode = FBl");
        pinMode(DPIN_PWR, OUTPUT);
        digitalWrite(DPIN_PWR, HIGH);
        digitalWrite(DPIN_DRV_MODE, HIGH);
      }
>>>>>>> rolled back because of bugs
      break;
    case MODE_PRE_PROGRAM:
      pinMode(DPIN_PWR, OUTPUT);
      digitalWrite(DPIN_PWR, HIGH);
      digitalWrite(DPIN_DRV_EN, LOW);
      break;
    case MODE_PROGRAM:
      pinMode(DPIN_PWR, OUTPUT);
      digitalWrite(DPIN_PWR, HIGH);
      Serial.println("Mode = Prog");
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











