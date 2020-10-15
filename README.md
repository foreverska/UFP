# Universal Front Panel

The front panel of a radio in this case refers to the user interface portion of a radio, the display and user inputs.  The core drive of this project is HAM radio where one may build custom radios with various digital tuning methods.

Universal Front Panel is built around the TI TM4C123  and it attempts to unmarry the pieces of the interface.  The core logic exists as a resuable portion and is linked at compile time to various forms of interface.  This enables a radio builder who for instance just wants a bigger display to merely write code for the display.

## Current Pieces

### Display

* ssd1306 OLED character display

### Input

* Single Knob mode based on native Quadrature Encoder interface

### PC Control

* Native USB->Serial Emulation with TS-480 Protocol Emulation

### Power Monitoring

* FET switched resistor divider (Battery Voltage)

### Radio

* SI514 Oscillator *[WIP-BROKEN]*

### Settings Storage

* 24xx256 EEPROM *[WIP-UNK]*

