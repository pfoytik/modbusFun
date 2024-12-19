# modbusFun
simple tests for modbus comm protocol

## Setup
- `sudo apt-get install libmodbus-dev`
- `pip install pymodbus`

## Run
use 2 terminals

term1: 
 -`gcc modbus_server2.c -o modbus_server -I/usr/include/modbus -lmodbus -lm`
 - `gcc -o readArduino readArduino.c`
 - `gcc -o combined_app combined_app.c -lpthread -lmodbus -lm`
 - `sudo ./modbus_server`
 - to quit `ctrl+c`
  
term2: 
- `ptyon testScript.py`
- to quit `ctrl+c`

