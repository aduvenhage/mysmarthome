# My Smart Home
Just another little project to keep track of home automation, camera and security stuff.

## Arduino / sensor_xs
### Focus
- LoRa based wireless sensors (sensor_xs)
- LoRa base station (sensor_app_1) 

### Arduino code setup
_NOTES:_ Remember to copy arduino_xs/libraries to your arduino IDE library folder.

### Use IDE version 1
https://learn.adafruit.com/adafruit-feather-32u4-radio-with-lora-radio-module/setup

### Extra libs
crc: https://github.com/RobTillaart/CRC (install via arduino IDE)
radio: https://learn.adafruit.com/adafruit-feather-32u4-radio-with-lora-radio-module/using-the-rfm-9x-radio

### Install arduino feather/lora board
Based on Adafruit Feather 32u4 + LoRa 868mhz
https://learn.adafruit.com/adafruit-feather-32u4-radio-with-lora-radio-module/using-with-arduino-ide

### LoRa Alarm App (base station)
Based on https://github.com/Makerfabs/Makerfabs_MaLora/tree/main/06MOS4, with custom firmware
