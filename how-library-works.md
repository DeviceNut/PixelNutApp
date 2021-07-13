How the PixelNutApp Library Works
===============================================================

This library is unusual in that it is simply a collection of ".h" files, which the parent application includes with the "#include PixelNutApp.h" statement, as opposed to being a self-contained set of classes that are separately compiled.

The main reason for this is to allow greater control over what actually gets compiled into an application, allowing smaller executables, which was necessary to be able to use processors that only have less than 32KB of available code space, without the extensive use of .extern files.

The means of gaining this control is to include all of the code into the main application, preceeded by #define statements that enable the selective compilation of various portions of code.

The "main.h" file has the main 'setup()' and 'loop()' routines, and is the heart of the application. These routines, in turn, call common control and update functions, implemented in the other files.

Both of these functions can be overriden with a #define so that you can replace them with your own custom functionality, without having to edit the files in the library itself. This provides a powerful means of extending the functionality provided here.


Built-in Classes
---------------------------------------------------------------

There are two defined classes in this library: 'CustomCode' and 'AppCommands'.

The former doesn't do anything by default, but allows adding custom functionality to the 'setup()' and 'loop()' routines without having to override the entire function, and is used by the bluetooth code in 'ble-bluefruit.h, and the wifi code in 'wifi-softap.h' and 'wifi-msqtt.h'. These handle communications with an external phone application client. It is also extended by code in 'segments.h' to provide support for multiple logical segments.


Main Control #defines
---------------------------------------------------------------

Here are some of the main #define statements that determine important functionality of the application. All of these defines are listed in the "keywords.txt" file.

'PIXEL_COUNT' sets the number of pixels to be displayed, and "PIXEL_OFFSET' shifts the drawing window around the pixel strip.

'STRLEN_PATTERNS' sets the size of the 'cmdStr' buffer used to hold the pattern strings, and MUST be large enough for the longest effect pattern defined internally, or to receive patterns sent from an external client.

'NUM_PLUGIN_LAYERS' and 'NUM_PLUGIN_TRACKS' set the size of the stacks used in the PixelNutLib Library, and must be at large enough for the effect patterns being used. If multiple strands are used, then 'LAYER_TRACK_COUNTS' must be defined as '{ number_of_layers, number_of_tracks, 0 }'.

'CUSTOM_PATTERNS' is set to 1 when defining internal patterns, whether because there is no external client, or because you're adding additional patterns. 'EXTERN_PATTERNS' is normally set to 1 if using an external client (which has built-in patterns). Set 'EXTERN_PATTERNS' to 0 when you don't want to use these external patterns, and use only internal custom ones. Use 'PATTERN_OVERRIDE' to override all of the above and only use specific patterns defined by your application (see FireFly example).

'SEGMENT_COUNT' is set to the number of logical (on the same single strand - set STRANDS_MULTI to 0) or physical (multiple strands - set STRANDS_MULTI to 1). Logical segments are fully supported in 'segments.h'. This is used in the SaturnHat example to allow use of pixel rings and a strand connected together on the same output pin. Multiple physical strands can be supported by extending this library.

Set 'EEPROM_FORMAT' to 1 once to format (clear to 0) the entire EEPROM area, then back to 0 to enable normal operation: the storing of various user settings across power cycles.


Communications #defines
---------------------------------------------------------------

Currently, there is support for bluetooth using Adafruit hardware ('BLE_BLUEFRUIT') and on the ESP32 ('BLE_ESP32'), and wifi on the ESP32 using either the SoftAP ('WIFI_SOFTAP') or MQTT ('WIFI-MQTT') handlers. These in turn set 'EXTERNAL_COMM', and either 'BLE_COM' or 'WIFI_COM'.


Override/Extend #defines
---------------------------------------------------------------

The following are used to allow overriding various parts of the standard application for implementing custom features. 'PIXENGINE_OVERRIDE' and 'SHOWPIX_OVERRIDE' allows overriding variables in 'globals.h', 'APPCMDS_EXTEND' allows extending application commands (for multi-segment support for example). 'MAIN_OVERRIDE' is necessary if you want to provide your own 'main' and 'loop' commands (as is in 'xmain.h' for example). Finally, 'CUSTOM_CODE' is used to provide your own implemtation of the 'CustomCode' class, or extend the ones use for communications.


Hardware Controls #defines
---------------------------------------------------------------

'MAX_BRIGHTNESS' sets the maximum percentage that the user can achieve with any hardware control, useful for projects are powered from batteries, to extend the time before power is lost. See the file "bright.h" for that code.

'DELAY_OFFSET' is used to adjust the default delay/speed of the animations, and will raise or lower the entire range of delay presets. 'DELAY_RANGE' specifies the maximum range for the delay value. These are only used on products with physical controls.

This is used to compensate for processor speed and the number of pixels that are being displayed (which takes longer). See the file 'delay.h' for that code.


Hardware Pin #defines
---------------------------------------------------------------

Only for products with physical controls:

Each 'DPIN_' and 'APIN_' definition causes code to handle the associated piece of hardware to be included in the compilation.

There are usually 2 different routines that need to be called for each type of hardware control: a 'Setup' routine, and a 'Check' routine, the former being called once at setup() time, and the latter during each iteration of 'loop()'.

If the associated control code is not being compiled for a given application, these calls resolve to empty functions (which are then eliminated by the linker during optimization).

For example, 'SetupColorControls()' and 'CheckColorControls()' do nothing if the symbols 'APIN_HUE_POT' and 'APIN_WHITE_POT' are not defined, which would indicate that aren't any pots being used to adjust the color properties.


Extended Plugin #defines
---------------------------------------------------------------

'PLUGIN_SPECTRA', and 'PLUGIN_PLASMA' enables additional effect plugins that can be used in your patterns. If 'MATRIX_STRIDE' is more than 1 then the Spectra plugin assumes you are using a matrix of pixels with the size of each column equal to that value.


Extended Application #defines
---------------------------------------------------------------

'XAPP_FIREFLY' enables the extended application 'FireFly' - see the example for more details.

To support multiple physical strands of pixels, you must first define 'XMAIN_ENABLE', 'APPCMDS_EXTEND', 'PIXENGINE_OVERRIDE', 'SHOWPIX_OVERRIDE' to use the code in 'xmain.h. Then set the following: 'PIXEL_COUNTS' to an array of the number of pixels in each strand, 'PIXEL_PINS' to an array of the pins driving the strand, 'PIXEL_DIRS' to an array of direction indicators (1=up, 0=down).
