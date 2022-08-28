# My Smart Home
Just another little project to keep track of home automation, camera and security stuff.

## Focus
- LoRa based wireless sensors 

# Setup
## Arduino code setup
_NOTES:_ Remember to copy arduino_xs/libraries to your arduino IDE library folder.

### Use IDE version 1
https://learn.adafruit.com/adafruit-feather-32u4-radio-with-lora-radio-module/setup

### Install feather/lora board
https://learn.adafruit.com/adafruit-feather-32u4-radio-with-lora-radio-module/using-with-arduino-ide

### Install radio
https://learn.adafruit.com/adafruit-feather-32u4-radio-with-lora-radio-module/using-the-rfm-9x-radio

### Extra libs
crc: https://github.com/RobTillaart/CRC (install via arduino IDE)

# Hardware
## LoRa sensor: Arduino XS
- based on Adafruit Feather 32u4 + LoRa 868mhz

## LoRa Alarm App (base station)
- based on https://github.com/Makerfabs/Makerfabs_MaLora/tree/main/06MOS4
- with custom firmware
