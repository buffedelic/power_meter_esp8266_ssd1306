# Power Meter esp8266

## Hardware

* ESP8266, ESP-12f
* SSD1306 i2c oled
* OneWire Counters, DS2423P

### Wiring

* Uses "standard" arduino i2c pins 4 and 5.
* IO0 is used for 1wire bus and needs a 0,8 to 1,5 kOhms pullup resistor to 3,3v.

### Credits

* Joe Bechter for the [DS2423P](https://github.com/jbechter/arduino-onewire-DS2423) implementation
