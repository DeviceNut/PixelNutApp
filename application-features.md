PixelNutApp Application Features
===============================================================

The following describes some of the features of the applications included as part of the PixelNutApp Library.


LED Error/Status Notifications
---------------------------------------------------------------

This error/status LED is either the one that is on the processor board for board applications, or an external one present on the some of the box applications, and is always red. Note that Hat applications don't implement this feature.

It is used to give a quick visual confirmation that everything is ok when powered up after 'setup()' is finished, and to identify the source of any problems that occur while running due to either programming mistakes or hardware failures.

When first powered up, the LED turns on, and stays on while 'setup()' is running. Before exiting 'setup()', the LED gives one quick blink to indicate that it has successfully completed.

If however there was an error during 'setup()', then the LED will repetitively blink to indicate that it failed and cannot continue.

There are 2 possible errors during 'setup()':

1) 1 long blink, 1 short blink
No preset pattern strings could be found in the flash memory. This only applies to products with manual controls, instead of being controlled with bluetooth.

2) 2 long blinks, 3 short blinks
Not enough memory available to allocate internal Engine buffers.

While running 'loop()' the LED is normally off, except when an error is encountered. Unlike during 'setup()' however, errors encountered while running the loop blink to give a notification but execution continues afterwards.

There are 3 possible errors of this sort:

1) 2 long blinks, N short blinks
This is an error from the PixelNutEngine, where N is:
   N=1 an invalid value was used in a command
   N=2 a command was unrecognized or otherwise invalid
   N=3 there was not enough memory to execute the command

If you are creating your own pattern strings, you might see these types of errors if the command string is malformed somehow.

2) 3 long blinks, N short blinks
This is an error from the BluefruitStrs Library, where N is:
   N=1 an error from the Bluefruit firmware
   N=2 invalid packet data (hardware/timing issues?)
   N=3 software error of some kind, or power issue
   N=4 never received the 'OK' acknowledgement

3) 4 long blinks, 1 short blink
This indicates an invalid command string was received over bluetooth communications, probably an issue with the mobile application.

4) There is one other type of status notification: if the #define value 'EEPROM_FORMAT' is set, then the EEPROM is cleared to zeros, and you will see continuous short blinking when it has completed.

The code that performs the LED blinking is found in 'leds.h'.


Use of Flash to Store Patterns
---------------------------------------------------------------

Flash memory is where the application code is stored when it is burned in using your toolchain or Arduino IDE. It is also where the built-in patterns are stored as well for configurations with physical controls, instead of into the limited amount of data space in many Arduino processors.

The file 'MyPatterns.h' defines the pattern strings, and code in the files 'select.h' and 'patterns.h' handle their selection and retrieval.


Use of EEPROM to Store Default Pattern
---------------------------------------------------------------

The hardware system must support EEPROM (or flash memory that is made to appear as such by an underlying library), as various applications settings are stored there and retrieved upon startup.

These settings include the initial preset effect pattern, and for applications that don't have physical controls, the current values for the brightness, animation delay, and all the externally set properties.

The code for this resides in 'flash.h'.


Modifying Pattern Animations with Physical Controls
---------------------------------------------------------------

Both buttons and pots (potentiometers) can are used as user input controllers for modifying effect patterns during the animation.

There are example applications that use only buttons, or both buttons and pots (potentiometers), or ones that only use bluetooth for control.

For physical control applications, they all use buttons to select the pattern and trigger actions (discussed in the next section), whereas either buttons or pots are used to adjust brightness and the animation speed.

For more advanced applications the color hue, color whiteness, and pixel count properties could be adjusted with pots as well.

One final thing to talk about is how button presses are used to select, modify, and save effect patterns. There are two distinct options available, depending on whether 1, 2, or 3 buttons are utilized.

If only a single button is used (only 'DPIN_PATTERN_BUTTON' is defined), then it has the following functions:

A) Single Press
This selects the next effect pattern to be run, returning to the first one after reaching the end (there are 13 presets).

B) Double Press
Similar to the standard double-click used on computers (press, release, then press again with less than 1/4 second delay, then release again), this causes an effect trigger on all enabled effect layers. The amount of force associated with the trigger depends on the amount of time the button is held down the second time, up to 1 second.

C) Long Press
This means pressing and holding the button down for greater than 1 second, then releasing. This will save the current pattern as the default when first powered on.

Add an additional button (#define 'DPIN_PAT_BUTTON_PREV') to allow choosing the previous pattern to the current one in the sequence of presets. It does the same thing as far as the B) and C) functions are concerned.

Finally, you can add a third button ('DPIN_TRIGGER_BUTTON') that implements function B) when pressed, and then all of the buttons no longer do anything when double pressed. This makes using the triggering function easier (just a single press, hold for force value, then release), and is used in some of the box products.


Triggering Animation Actions
---------------------------------------------------------------

Using either a dedicated button, or by double pressing the pattern select button, the user can trigger actions on the currently running animation. 

The concept of triggering and its use of a force is covered in the documentation for the PixelNutLib Library in the document 'library-design.md'.

It is a means of causing some action to be taken on the animation, the scale of which depends on its associated force value.

What this actually does depends on how what effects are being used in the pattern, and thus what the code for the associated plugin does with that triggering force.

For example, the very first of the built-in patterns (called RAINBOW_RIPPLE in 'MyPatterns.h') constantly changes colors that "ripple" down the pixel strip. When triggered by the user, the amount of force determines how fast the colors change.


Getting Debug Output
---------------------------------------------------------------

The Serial Monitor can be used to display debug output messages for applications, except if the processor doesn't have enough memory to support it (the CometHat and MeteorHat for example).

Debug mode is enabled by declaring: "#define DEBUG_OUTPUT 1" in the main .INO file. The maximum length of the string used to format the output (using printf style format strings) is set to larger than the longest pattern string, which is set with the #define STRLEN_PATTERNS.

There are many output statements already present in the code to explain error conditions and indicate when control settings have changed, and you can easily add more as needed for your purposes.

There are the two debug macros used: 

'DBG(<code>)' 
Only compiles in the <code> when debug mode is enabled.

'DBGOUT((F(<format_str>), arg1, arg2, ...))' 
What causes the <format_str> to be parsed using the substitution arguments 'arg1' etc, to create an output string sent to the Serial Monitor.

(Note: the 'F()' macro is used to avoid taking up precious data space with the debug format strings, implemented on AVR processors, but not others.)

One other thing to keep in mind when using the debug mode is that the application will wait for up to 5 seconds for the Serial Monitor to be attached, so as not to miss the beginning output. So you'll want to turn debug mode off when not using it to avoid an unnecessary pause each time you power up.

All of the debug related code is in 'dbgerr.h'.
