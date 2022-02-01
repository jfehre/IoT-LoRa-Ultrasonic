# IoT LoRa Ultrasonic

## Introduction

The goal of this arduino project is to create an IoT Device which measures the distance and uploads data over [The Things Network](https://www.thethingsnetwork.org/) to the [OpenSenseMap](https://opensensemap.org/). Between measurements the microcontroller goes into deep sleep. The focus lies on an low power consumption solution, so the sensor is able to measure data over several months.  
For measurement, this script supports two ways of communicating with an ultrasonic distance sensor:


- **UART Serial:** (tested with the [waterproof ultrasonic sensor SKU (A02YYUW)](https://wiki.dfrobot.com/A02YYUW%20Waterproof%20Ultrasonic%20Sensor%20SKU:%20SEN0311) from dfrobot.)
- **Analog (Trig/Echo):**  (tested with the AJ-SR04M and HC-SR04 Sensors)


**TTGO ESP32 LoRA:** The first approach happened with an [TTGO ESP32 LoRa](http://www.lilygo.cn/prod_view.aspx?TypeId=50003&Id=1141&FId=t3:50003:3) board. However the power consumption of the board during deep sleep is too high (10mA). Furthermore the sensors always drained power (even during deep sleep), but this could be solved by powering the sensor with a GPIO Pin.  
-> see subfolder [TTGO-ESP32-Ultrasonic](/TTGO-ESP32-Ultrasonic).

**HelTec CubeCell:** As a result of the high power consumption during deep sleep a [Heltec CubeCell](https://heltec.org/project/htcc-ab01/) microcontroller is used, which should need 2µA during deep sleep.  
-> see subfolder [Heltec-CubeCell-Ultrasonic](/Heltec-CubeCell-Ultrasonic).


## Getting Started


### 1. Backend Setup

Whatever microcontroller or ultrasonic sensor is used, the backend setup is the same.

#### 1.1 The Things Network
1. If you aren't already registered, [set up a The Things Network account](https://account.thethingsnetwork.org/register)
2. Create a new Application in TTN [Applications Console](https://console.thethingsnetwork.org/applications)
3. Create a new end device and set following parameters: 
    * Frequencyplan -> depends on your location
    * LoRaWAN version -> This depends on the version set in the [Arduino-LMIC](https://github.com/mcci-catena/arduino-lmic) library. Default is MAC V1.0.3
    * Other parameters can be generated
4. In your Application select **Payload Formats** and set it to `Cayenne LPP`
5. For the integration with [OpenSenseMap](https://opensensemap.org/), select **Integrations** in your Application and create a new **Custom webhook**
6. Choose a unique 'Webhook ID' and use JSON as the 'Webhook format'. The 'Base URL' should be `https://ttn.opensensemap.org/v3`. It is important to enable 'Uplink message'. The remaining fields can be left out ([See here](https://forum.sensebox.de/t/cayenne-ttnv2-3-nach-osem-functioniert-nicht/1215/7)).

#### 1.2 OpenSenseMap
1. If you aren't already registered, [set up a OpenSenseMap account](https://opensensemap.org/register)
2. In your **Dashboard** create a 'New senseBox'
3. Fill in the appropriate data like 'name', 'exposure type' and 'location'
4. Select 'Manual configuration' for your **Hardware** and 'Add sensor' with Phenomenon: `distance`, Unit: `mm` and Type: `ultrasonic`.
5. In **Advanced** choose 'TheThingsNetwork-TTN' and add `Cayenne LPP (beta)` as 'Decoding Profile'. Furthermore add your 'TTN Application-ID' and 'TTN Device-ID' from your [TheThingsNetwork application](https://console.thethingsnetwork.org/applications).
6. In **Decoding Options** choose 'Cayenne LPP Phenomenon' as `Illumination` (to support floats with uint16) and if not already set, the 'Cayenne LPP Channel' to `1`.

### 2. Configure Microcontroller
This step depends on the microcontroller you are using. Please refer to the corresponding readme files:  

1. [TTGO-ESP32](/TTGO-ESP32-Ultrasonic)
2. [Heltec-CubeCell](/Heltec-CubeCell-Ultrasonic)

## Contribution

You are more than welcomed to contribute to this project by trying out other microcontroller or share your experience :)



