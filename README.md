# Distance Measurement with TTGO ESP32 and ultrasonic sensor

This is an Arduino projekt for the [TTGO ESP32 LoRa](http://www.lilygo.cn/prod_view.aspx?TypeId=50003&Id=1141&FId=t3:50003:3) board in combination with an [waterproof ultrasonic sensor SKU (A02YYUW)](https://wiki.dfrobot.com/A02YYUW%20Waterproof%20Ultrasonic%20Sensor%20SKU:%20SEN0311) from dfrobot.

It should read the measurement of the sensor and send the data to [The Things Network](https://www.thethingsnetwork.org/).

## 1. Setup
1. Install the [ESP32 Core for Arduino](https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/boards_manager.md) (Installation with Boards Manager)
2. Install the [LoRa](https://github.com/sandeepmistry/arduino-LoRa) library which supports the SX1276 LoRa Chip
3. Upload sketch to your board by using the `TTGO LoRa32-OLED-V1` Board

## 2. Wiring
1. Connect `GND` of Sensor to `GND` of Board
2. Connect VCC of Sensor to either `3V3` or `5V`
3. Connect `RX` of Sensor to PIN `17 (U2_TXD)` 
4. Connect `TX` of Sensor to PIN `16 (U2_RXD)`

You should also be able to use the other UART 1 connection of the board (Unforunately I'm not sure which PINs belong to the UART 1)


### [Sensor Information](https://wiki.dfrobot.com/A02YYUW%20Waterproof%20Ultrasonic%20Sensor%20SKU:%20SEN0311)
### TTGO ESP32 LoRa Pinout:
![ESP Pinout](images/esp_pinout.jpeg)


## 3. List of Libraries
- [ESP32 Core for Arduino](https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/boards_manager.md) (Installation with Boards Manager)
- [LoRa](https://github.com/sandeepmistry/arduino-LoRa)

