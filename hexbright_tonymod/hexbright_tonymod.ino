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
 v1.6  Dec 29, 2012: Tweaked fast blink and pre-dazzle driver level to avoid brightness change on button release.
 
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
#define DPIN_RLED_SW            2 // Switch and red LED. Can sense button presses when the pin is on input mode, and can light the red LED when the pin is in output mode.
#define DPIN_GLED               5 // Green LED
#define DPIN_PWR                8 // Set to output and bring high to make the atmega168 stay on.
#define DPIN_DRV_MODE           9 // Main LED driver power level. Valid values are digital high or low.
#define DPIN_DRV_EN             10 // Main LED. Can use digitalWrite for on or off, or analogWrite 0-255 for native PWM.
#define APIN_TEMP               0 // Temperature pin
#define APIN_CHARGE             3 // Charge controller status pin. Registers either off, charging, or charged.
// Modes
// MODE_OFFHOLD and PRE_ modes exist to catch long presses
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
byte mode = 0; // current mode
byte skiptomode = 255; // used for mode skipping; if set anywhere in the loop, the light will go to that mode in the next loop. 255 is used as a null value in this case.
boolean hml = false; // true for high-medium-low operation, false for low-medium-high operation
boolean lowskip = false, medskip = true, highskip = false, dazskip = false, fblskip = true, sblskip = false; // These are true if those modes are skipped, false if they are not
boolean mprog = false; // HexBright enters mode skip programming if true, is normally set true by a 0.5 second press from HML/LMH select mode, and then set false by the end of programming.
unsigned long btnTime = 0; // Button hold time
boolean btnDown = false; // Button status
boolean fadeDown = false; // Direction for charging LED fade
int fadeCount = 0; // LED level for charging and HML/LMH select fade
int fadeWait = 0; // Loop counter to set fade speed for green LED
int fadeOffWait = 0; // Loop counter for off time between fades for green LED
boolean gledOff = false; // Set between green LED fades to have it stay off for a time
boolean dazzle_on = true; // Tracking variable for dazzle
long dazzle_period = 100; // another tracking variable for dazzle

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

  // Read settings from EEPROM
  if (EEPROM.read(0) == 1) hml = true; // Only set variables true on 1 value. All other values set the variables false.
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
      // digitalWrite(DPIN_GLED, (time&0x0100)?LOW:HIGH); // Old charging status code

      // Green LED fade code
      if (!gledOff) // go to wait mode if gledOff is set
      {
        if (!fadeDown) // figure out if we're fading up
        {
          if (fadeWait == 35) // wait 35 loops between fade steps
          {
            analogWrite(DPIN_GLED, fadeCount++); // increment LED brightness
            if (fadeCount == 255) // if fully bright, start fading down
            {
              fadeDown = true;
            }
            fadeWait = 0; // reset fade wait counter
          }
          else
          {
            fadeWait++; // increment fade wait counter
          }
        }
        else if (fadeDown) // if we're fading down
        {
          if (fadeWait == 25) // wait 25 loops between fade steps (looks better on the way down for some reason)
          {
            analogWrite(DPIN_GLED, fadeCount--); // decrement LED brightness
            if (fadeCount == 0) // if off, send digital low for true off, then set gledOff to wait for a time before next fade
            {
              fadeDown = false; 
              gledOff = true;
              digitalWrite(DPIN_GLED, LOW);
            }
            fadeWait = 0; // reset fade wait counter
          }
          else
          {
            fadeWait++; // increment fade wait counter
          }
        }
      } 
      else if (gledOff && fadeOffWait < 2000) // this will be true if we're in fade off wait mode
      {
        fadeOffWait++; // increment fade off wait counter
      }
      else if (gledOff && fadeOffWait == 2000) // waited long enough, get to the next fade
      {
        fadeOffWait = 0;
        gledOff = false;
      }

    }
    else if (chargeState > 768) // High - charged
    {
      digitalWrite(DPIN_GLED, HIGH);
    }
    else // Hi-Z - shutdown - not charging
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
    if(mprog)digitalWrite(DPIN_GLED, !lowskip);
    break;
  case MODE_MED:
    if(mprog)digitalWrite(DPIN_GLED, !medskip);
    break;
  case MODE_HIGH:
    if(mprog)digitalWrite(DPIN_GLED, !highskip);

    break;
  case MODE_SBL:
  case MODE_PRE_SBL:
    if(mprog)digitalWrite(DPIN_GLED, !sblskip);

    digitalWrite(DPIN_DRV_EN, (time%300)<75); // 300ms loop, 75 ms on
    break;
  case MODE_DAZZLE:
  case MODE_PRE_DAZZLE:
    if(mprog)digitalWrite(DPIN_GLED, !dazskip);  // Dazzle code from digitalmisery on GitHub

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

    digitalWrite(DPIN_DRV_EN, (time%75)<8); // 75 ms loop, 8 ms on
    break;
  case MODE_PROGRAM: // LMH/HML program mode, most of this code governs the fading of the green LED to demonstrate HML and LMH modes
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
  if (skiptomode != 255) { // check for mode skip and act if necessary
    newMode = skiptomode; 
    skiptomode = 255;
  }
  byte newBtnDown = digitalRead(DPIN_RLED_SW);
  switch (mode) // button actions are determined here. make sure to use a pre mode if you are acting on a button hold.
  {
  case MODE_OFF:
    if (hml && btnDown && !newBtnDown && (time-btnTime)>20) // button tap
      newMode = MODE_HIGH;
    if (!hml && btnDown && !newBtnDown && (time-btnTime)>20)
      newMode = MODE_LOW;
    if (btnDown && newBtnDown && (time-btnTime)>500) // button hold
      newMode = MODE_PRE_DAZZLE;
    break;
  case MODE_PRE_LOW:
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
      lowskip = !lowskip;
    if (mprog && hml && btnDown && newBtnDown && (time-btnTime)>500)
      newMode = MODE_PRE_DAZZLE;
    if (mprog && !hml && btnDown && newBtnDown && (time-btnTime)>500)
      newMode = MODE_PRE_MED;
    break;
  case MODE_PRE_MED:
    // This mode exists just to catch this button release.
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
      medskip = !medskip;
    if (mprog && hml && btnDown && newBtnDown && (time-btnTime)>500)
      newMode = MODE_PRE_LOW;
    if (mprog && !hml && btnDown && newBtnDown && (time-btnTime)>500)
      newMode = MODE_PRE_HIGH;
    break;
  case MODE_PRE_HIGH:
    // This mode exists just to catch this button release.
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
      highskip = !highskip;
    if (mprog && hml && btnDown && newBtnDown && (time-btnTime)>500)
      newMode = MODE_PRE_MED;
    if (mprog && !hml && btnDown && newBtnDown && (time-btnTime)>500)
      newMode = MODE_PRE_DAZZLE;
    break;
  case MODE_PRE_DAZZLE:
    // This mode exists just to catch this button release.
    if (btnDown && !newBtnDown)
      newMode = MODE_DAZZLE;
    if (!mprog && btnDown && newBtnDown && (time-btnTime)>4000)
      newMode = MODE_PRE_PROGRAM;
    break;
  case MODE_PRE_PROGRAM:
    // This mode exists just to catch this button release.
    if (btnDown && !newBtnDown)
      newMode = MODE_PROGRAM;
    break;
  case MODE_PROGRAM:
    if (btnDown && !newBtnDown && (time-btnTime)>20)
    {
      hml = !hml;
      Serial.print("HML now ");
      Serial.println(hml);
    }
    if (btnDown && newBtnDown && (time-btnTime)>500)
    {
      if (hml) newMode = MODE_PRE_HIGH; 
      else newMode = MODE_PRE_LOW;
      digitalWrite(DPIN_GLED, LOW);
      mprog = true; // enter mode skip program mode
    }
    break;
  case MODE_OFFHOLD:
    // This mode exists just to catch this button release.
    if (btnDown && !newBtnDown)
      newMode = MODE_OFF;
    break;
  case MODE_PRE_SBL:
    // This mode exists just to catch this button release.
    if (btnDown && !newBtnDown)
      newMode = MODE_SBL;
    break;
  case MODE_SBL:
    if (!mprog && btnDown && !newBtnDown && (time-btnTime)>20)
      newMode = MODE_OFF;
    if (!mprog && btnDown && newBtnDown && (time-btnTime)>500)
      newMode = MODE_OFFHOLD;
    if (mprog && btnDown && !newBtnDown && (time-btnTime)>20)
      sblskip = !sblskip;
    if (mprog && btnDown && newBtnDown && (time-btnTime)>500) // last programming step, so check if hml/skip variables match EEPROM and write if necessary, reset all fade counter variables (just in case we're plugged in), then turn the light off.
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
    // This mode exists just to catch this button release.
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

  // Do the mode transitions (one time execution on mode change)
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
      if(!mprog && lowskip && hml) { // skip mode if skip variable is true
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
    case MODE_PRE_DAZZLE:
      digitalWrite(DPIN_DRV_MODE, HIGH);
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







