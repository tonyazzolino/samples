HexBright Demo Code 
=======================

hexbright_tonymod
-----------------
This software is a modification of the factory program. Button presses cycle
through either off, low, medium, and high modes or off, high, medium and low modes
depending on what mode the HexBright is in. A half second press from off will go to Dazzle,
whereas taps from there will go to fast blink, slow blink, and off modes. Mode skipping is also
enabled in this sketch.
To enter program mode, press and hold the button from the off mode. The light will go
into Dazzle mode and then it will shut off after 4 seconds. When you release the button, the green
LED on the back of the HexBright will either fade up for LMH mode or fade down for HML mode.
When the green LED indicates the mode you want, press and hold the button for a half second to
go to mode enable selection. In mode enable selection, the HexBright will first go to low or high,
depending on the LMH/HML setting. The green LED will be lit if the mode is enabled and out if it is
disabled. A tap of the button toggles the enabled/disabled status of the current mode, while a
a 0.5 second press moves to the next mode. After slow blink mode, the light will save its settings
if necessary, then turn off. 

hexbright_factory
-----------------
This is the software that ships with the Hexbright Flex.  Button presses cycle
through off, low, medium, and high modes.  Hold down the button while off for 
blinky mode.

hexbright4
---------------------
Fancier than the factory program, but designed for everyday usability.  Button
presses cycle through off, low and high modes.  Hold the light horizontally,
hold the button down, and rotate about the long axis clockwise to increase
brightness, and counter-clockwise to decrease brightness- the brightness sticks
when you let go of the button.  While holding the button down, give the light a
firm tap to change to blinky mode, and another to change to dazzle mode.

hexbright_demo_morse
--------------------
Flashes out a message in morse code every time you press the button.  Nothing 
else.  The message and speed are easy to change- you can see and change both 
in the first lines of code.

hexbright_demo_taps
-------------------
Hold the button down, and with your other hand firmly tap on the light.  Tap
some more times, and let go of the button.  The exact sequence of taps will
be recorded and then played back as flashes until you press the button again
to turn off.

hexbright_demo_momentary
------------------------  
Light turns on only while the button is being held down.  That's it.

hexbright_demo_dazzle
---------------------
Light runs in dazzle mode only as long as the button is being held down.

hexbright_demo_fades
--------------------  
Hold the button down, and light fades up and down.  Let go, and it holds the 
current brightness.  Another press to turn off.
