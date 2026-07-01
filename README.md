# E-book keychain
### A small E-book with one word at time reading with audiobook 
I have always needed something i can read and hear stories without using my phone at all and i want it should me able to carry anywahere easily and english is my second language so i get struck on some words so i need quick access to the meaning also so i made this.
This is a small esp32 based e-book with audiobook small enough to be in a keychain:
#### Features
- one word at a time, easy and fast reading
- one click dictionary, eassy dictionary access without losing anything
- audiobook, to hear your storys
- easy file sharing, with the help of captive web

# Photo 📷

case
- ![pic1](/images/cad1.png)
- ![pic2](/images/cad2.png)
- ![pic3](/images/cad3.png)

# Hardware

Electronics you need:
- ESP32-C6-LCD-1.47
- CJMCU-4344 CS4344 Digital To Analog Stereo Audio Conversion Module
- 502030 500mah battry
- tactile switch smd 5x5x1.5
- 3x6x3.5MM Side Press Type SMT Tactile Push Button Switch
- some wires 

# PCB
here are the wiring and PCB digram:
- 
- ![pcb1](/images/pcb1.png)
- ![pcb2](/images/pcb2.png)
### GPIO connections


|Button|	GPIO|	Function|
|-----|-----------|-------|
|Up	|GPIO1	|Navigate Up / Increase|
|Down|GPIO2|Navigate Down / Decrease|
Left|GPIO3|Navigate left/Previous
Right|GPIO10|Navigate right /Next
OK|GPIO11|Select / Play / Pause
Power|GPIO17|Sleep / Wake
Dictionary|GPIO4|Open Dictionary
Back	|GPIO5	|Go Back


# Firmware
use platformio for uploading program to the esp32 

### Folder layout

```
frimware/
|--platformio.ini             
|--README.md                  
|--include/                   
│   |--config.h               
│   |--theme.h                
│   |--types.h                
│   |--ui.h
│   |--input.h
│   |--storage.h
   |--book_manager.h
│   |--reader.h
│   |--dictionary.h
│   |--audio_player.h
│   |--wifi_manager.h
│   |--settings.h
│   └-- User_Setup.h           
-- src                       
│   |--main.cpp              
│   |--ui.cpp
│   |--input.cpp
│   |--storage.cpp
│   |--book_manager.cpp
│   |--reader.cpp
│   |--dictionary.cpp
│   |--audio_player.cpp
│   |--wifi_manager.cpp
│   |__settings.cpp
|--scripts/
│   |__copy_user_setup.py     # platformio preuild
|__data/                     
    |--books/                 
    |__audio/ 
    |__ dict.txt/            
```

### SD card layout
```
SD root
|--books/                
│   |__#your .txt file will be here
|--audio/                # 
│   |__#your .mp3 wuill be here
|--dict.txt              
|--settings.cfg          
|--progress.cfg          
|__state.cfg             
```
# BOM (Bill of Material)
[BOM]( /doc/BOM.csv)

| item | quantity | price | link |
--------|---------|--------|---------|
|Esp32-c6-LCD|1|Rs.1135.00| [Electropi](https://www.electropi.in/waveshare-esp32-c6-147inch-display-development-board-172320-262k-color-160mhz-running-frequency-single-core-processor-supports-wifi-6-bluetooth-esp32-with-display)
|5x5x1.5mm Momentary Tactile Push Button Switch SMD| 8 | RS.80 |[Amazon](https://amzn.in/d/05bs0ZPJ)|
|3x6x3.5MM Side Press Type SMT Tactile Push Button Switch| 1| RS.5 ||
|CS4344 24Bit DAC Converter Stereo Audio Module| 1|RS.170|[Amazon](https://electropeak.com/cs4344-24bit-dac-converter-stereo-audio-module)
|502030 500mah battry|1|RS.199|https://amzn.in/d/0jcyAWPH|
