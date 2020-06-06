How the PixelNutApp Library Works
===============================================================

This library is unusual in that it is simply a collection of ".h" files, which the parent application includes with the "#include PixelNutApp.h" statement, as opposed to being a self-contained set of classes that are separately compiled.

The main reason for this is to allow greater control over what actually gets compiled into an application, allowing smaller executables, which was necessary to be able to use processors that only have less than 32KB of code space available.

The means of gaining this control is to include all of the code into the main application, preceeded by #define statements that enable the selective compilation of various portions of code.

The "main.h" file has the main 'setup()' and 'loop()' routines, and is the heart of the application. These routines, in turn, call common control and update functions, implemented in the other files.

Both of these functions can be overriden with a #define so that you can replace them with your own custom functionality, without having to edit the files in the library itself.


Built-in Classes

There are two defined classes in this library: 'CustomCode' and 'AppCommands'.

The former doesn't do anything by default, but allows adding custom functionality to the 'setup()' and 'loop()' routines without having to override the entire function, and is in fact used by the bluetooth code in 'bluetooth.h'.

The latter handles communications with an external client (phone app or browser) when using bluetooth or WiFi. It is called by routines in 'bluetooth.h' or "wifi.h", and is extended by code in 'segments.h' to provide support for multiple logical segments.


Main Control Defines

Here are some of the main #define statements that determine important functionality of the application. All of these defines are listed in the "keywords.txt" file.

'PIXEL_COUNT' sets the number of pixels to be displayed, and "PIXEL_OFFSET' shifts the drawing window around the pixel strip.

Applications that use bluetooth must set 'BLE_COMM' to 1. Applications that use WiFi must set 'WIFI_COMM' to 1.

'STRLEN_PATTERNS' sets the size of the 'cmdStr' buffer used to hold the pattern strings, and MUST be large enough for the longest effect pattern defined internally, or to receive patterns sent from an external client (over bluetooth).

'NUM_PLUGIN_LAYERS' and 'NUM_PLUGIN_TRACKS' set the size of the stacks used in the PixelNutLib Library, and must be at large enough for the effect patterns being used.

'CUSTOM_PATTERNS' is set to 1 when defining internal patterns (this is the default). Set 'BASIC_PATTERNS' to 1 to restrict the available plugins to the most basic ones to save on code flash space.

'STRAND_COUNT' is set to the number of physically separate pixel strands being used (default is 1). Support for more than 1 requires additional code that is not privided here.

'SEGMENT_COUNT' is set to the number of separate segments, whether or not they're on a single strand or not (default is 1). Using segments on a single strand is fully supported in 'segments.h'. This is used in the SaturnHat example to allow use of pixel rings and a strand connected together on the same output pin.

Set 'EEPROM_FORMAT' to 1 once to format (clear to 0) the entire EEPROM area, then back to 0 to enable normal operation: the storing of various user settings across power cycles.


Defines for Hardware Controls

'MAX_BRIGHTNESS' sets the maximum percentage that the user can achieve with any hardware control, useful for projects are powered from batteries, to extend the time before power is lost. See the file "bright.h" for that code.

'DELAY_OFFSET' is used to adjust the default delay/speed of the animations, and will raise or lower the entire range of delay presets. This is only used on products with physical controls.

This is used to compensate for processor speed and the number of pixels that are being displayed (which takes longer). See the file 'delay.h' for that code.


Hardware Pin Definitions

Only for products with physical controls:

Each 'DPIN_' and 'APIN_' definition causes code to handle the associated piece of hardware to be included in the compilation.

There are usually 2 different routines that need to be called for each type of hardware control: a 'Setup' routine, and a 'Check' routine, the former being called once at setup() time, and the latter during each iteration of 'loop()'.

If the associated control code is not being compiled for a given application, these calls resolve to empty functions (which are then eliminated by the linker during optimization).

For example, 'SetupColorControls()' and 'CheckColorControls()' do nothing if the symbols 'APIN_HUE_POT' and 'APIN_WHITE_POT' are not defined, which would indicate that aren't any pots being used to adjust the color properties.
