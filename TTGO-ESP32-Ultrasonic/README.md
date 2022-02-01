# TTGO ESP32

This folder contains the implementation with a [TTGO ESP32 LoRa](http://www.lilygo.cn/prod_view.aspx?TypeId=50003&Id=1141&FId=t3:50003:3).

## 1. Backend Setup

For the backend setup please follow the instructions on the [main readme](https://github.com/jfehre/IoT-LoRa-Ultrasonic)

## 2. Install Libraries in Arduino IDE
1. Install the [ESP32 Core for Arduino](https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/boards_manager.md) (Installation with Boards Manager)
2. Install the [LoRa](https://github.com/sandeepmistry/arduino-LoRa) library which supports the SX1276 Chip
3. Install [CayenneLPP](https://github.com/sabas1080/CayenneLPP) to package LoRa packets.
4. Install [Arduino-LMIC](https://github.com/mcci-catena/arduino-lmic) to send data over LoRaWan

## 3. ESP Setup
1. Copy the Template: `cp config.h.template config.h`. (This file is filled in with private data and should not be published)
2. Insert **Application EUI** (lsb), **Device EUI** (lsb) and **App Key** (msb) from the **Device Overview** in your [TheThingsNetwork application](https://console.thethingsnetwork.org/applications) to the `config.h` file. (Note: use the 'C-style' not Hex)
3. *Optional*: Change in `config.h` the way how to read sensordata (UART/A02YYUW or Trig/Echo)
4. *Optional*: Change the pins in `config.h`
5. *Optional*: Change cycle time in `config.h`
6. *Optional*: If you want to use a GPIO as power pin (only available for 3.3V sensor), enable it in `config.h` and define PWR_PIN. This should save more power during deep sleep, because some sensor always drain power.
7. Upload sketch to your board by using the `TTGO LoRa32-OLED-V1` Board from the [ESP32 Core for Arduino](https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/boards_manager.md)

#### Notes:

- Maybe you need to change the 'Region Configuration' of the [Arduino-LMIC](https://github.com/mcci-catena/arduino-lmic) library. To change it, go to the `lmic_project_config.h` file within the LMIC Arduino library ([See here](https://github.com/mcci-catena/arduino-lmic#configuration)).
- ~~When using **ABP Activation** instead of **OTAA Activation** the TTN application does not receive packages after a restart of the device. The option 'reset frame counters' in your TTN **Device Overview** helps ([see here](https://forum.sodaq.com/t/not-receiving-data-until-frame-counter-reset-in-ttn-console/632/3)).~~

## 4. Wiring of the Ultrasonic Sensor

### UART (A02YYUW)
1. Connect `GND` of Sensor to `GND` of Board
2. Connect VCC of Sensor to either `3V3` or `5V` (if using a power pin, connect it to the corresponding pin, default `21 (GPIO)`)
3. Connect `RX` of Sensor to PIN `17 (U2_TXD)` 
4. Connect `TX` of Sensor to PIN `16 (U2_RXD)`

You should also be able to use the other UART 1 connection of the board (Unforunately I'm not sure which PINs belong to the UART 1)

[Sensor Information](https://wiki.dfrobot.com/A02YYUW%20Waterproof%20Ultrasonic%20Sensor%20SKU:%20SEN0311)

### Echo/Trig sensors (AJ-SR04M and HC-SR04)
1. Connect `GND` of Sensor to `GND` of Board
2. Connect VCC of Sensor to either `3V3` or `5V` (if using a power pin, connect it to the corresponding pin, default `21 (GPIO)`)
3. Connect `Trig` of Sensor to PIN `13 (GPIO)` 
4. Connect `Echo` of Sensor to PIN `12 (GPIO)`

You can also use other GPIO pins which are not used internally by the board.

## 5. TTGO ESP32 LoRa Pinout:
![ESP Pinout](images/esp_pinout.jpeg)

## Information about Power Consumption


- Board: active -> 66mA : deep sleep -> 20mA

