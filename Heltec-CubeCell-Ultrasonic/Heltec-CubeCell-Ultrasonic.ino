/*
 * 
 * 
 * Codesnippets:
 *   - Heltec CubeCell examples LoRaWAN/CayenneLPP 
 *     https://github.com/HelTecAutomation/CubeCell-Arduino/tree/master/libraries/LoRa/examples/LoRaWAN/LoRaWan_CayenneLPP
 */

#include <softSerial.h>
#include "LoRaWan_APP.h"
#include <CayenneLPP.h>

// COPY the config.h.template and RENAME to config.h file in the same folder WITH YOUR TTN KEYS AND ADDR.
#include "config.h"

//sensordata
float distance = -1;

/*LoraWan channelsmask, default channels 0-7*/ 
uint16_t userChannelsMask[6]={ 0x00FF,0x0000,0x0000,0x0000,0x0000,0x0000 };

/*LoraWan region, select in arduino IDE tools*/
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;

/*LoraWan Class, Class A and Class C are supported*/
DeviceClass_t  loraWanClass = LORAWAN_CLASS;

/*the application data transmission duty cycle.  value in [ms].*/
uint32_t appTxDutyCycle = TX_INTERVAL * 1000;

/*OTAA or ABP*/
bool overTheAirActivation = LORAWAN_NETMODE;

/*ADR enable*/
bool loraWanAdr = LORAWAN_ADR;

/* set LORAWAN_Net_Reserve ON, the node could save the network info to flash, when node reset not need to join again */
bool keepNet = LORAWAN_NET_RESERVE;

/* Indicates if the node is sending confirmed or unconfirmed messages */
bool isTxConfirmed = LORAWAN_UPLINKMODE;

/* Application port */
uint8_t appPort = 2;
/*!
* Number of trials to transmit the frame, if the LoRaMAC layer did not
* receive an acknowledgment. The MAC performs a datarate adaptation,
* according to the LoRaWAN Specification V1.0.2, chapter 18.4, according
* to the following table:
*
* Transmission nb | Data Rate
* ----------------|-----------
* 1 (first)       | DR
* 2               | DR
* 3               | max(DR-1,0)
* 4               | max(DR-1,0)
* 5               | max(DR-2,0)
* 6               | max(DR-2,0)
* 7               | max(DR-3,0)
* 8               | max(DR-3,0)
*
* Note, that if NbTrials is set to 1 or 2, the MAC will not decrease
* the datarate, in case the LoRaMAC layer did not receive an acknowledgment
*/
uint8_t confirmedNbTrials = 4;

//Define Serial for UART connection
softSerial mySerial(TX_PIN, RX_PIN);
unsigned char data[4]={};
boolean newData = false;
boolean headerReceived = false;
byte dataIndex = 0;

/* Prepares the payload of the frame */
static void prepareTxFrame( uint8_t port )
{
  /*appData size is LORAWAN_APP_DATA_MAX_SIZE which is defined in "commissioning.h".
  *appDataSize max value is LORAWAN_APP_DATA_MAX_SIZE.
  *if enabled AT, don't modify LORAWAN_APP_DATA_MAX_SIZE, it may cause system hanging or failure.
  *if disabled AT, LORAWAN_APP_DATA_MAX_SIZE can be modified, the max value is reference to lorawan region and SF.
  *for example, if use REGION_CN470, 
  *the max value for different DR can be found in MaxPayloadOfDatarateCN470 refer to DataratesCN470 and BandwidthsCN470 in "RegionCN470.h".
  */
    CayenneLPP lpp(LORAWAN_APP_DATA_MAX_SIZE);
    lpp.addLuminosity(1, distance);
    lpp.getBuffer(), 
    appDataSize = lpp.getSize();
    memcpy(appData,lpp.getBuffer(),appDataSize);
}

///////////////////////////////////////////////////
// Function to read distance from different sensortypes (float distance saves distance in mm)

void getDistance() {

  if(ENABLE_PWR_PIN)
  {
    digitalWrite(PWR_PIN, LOW);
    delay(1000);
  }
  
  switch (SENSOR)
  {
    //read sensor over UART (sensortype A0YYUW)
    case A0YYUW:
    {
      mySerial.begin(9600);
      // delay so serial is available after begin
      delay(100);
      // flushing otherwise wrong data come in
      mySerial.flush();
      // delay so serial available after flush
      delay(100);
      while (mySerial.available() > 0 && newData == false) {
        if (headerReceived == false)
        {
          // wait for header byte (starting with 0xFF)
          if (mySerial.read() == 0xFF)
          {
            headerReceived = true;
            data[0] = 0xFF;
            dataIndex++;
          }
        }else 
        {
          // read data
          data[dataIndex] = mySerial.read();
          dataIndex++;
          if (dataIndex == 4)
          {
            dataIndex = 0;
            newData = true;
          }
        }
      }
      //calculate distance
      int sum;
      sum=(data[0]+data[1]+data[2])&0x00FF;
      if(sum==data[3])
      {
        distance=(data[1]<<8)+data[2];
        Serial.println(distance/10);
      }else Serial.println("ERROR");
    
      newData = false;
      headerReceived = false;
      // end serial to avoid power consumption in sleep mode
      mySerial.end();
      break;
    }
    //read sensor over TRIG/ECHO (sensortype ex. AJ-SR04M or HC-SR04)  
    case TRIG_ECHO:
    {
      digitalWrite(TRIG_PIN, LOW);  
      delayMicroseconds(5); 
  
      digitalWrite(TRIG_PIN, HIGH);
      delayMicroseconds(10); 
  
      digitalWrite(TRIG_PIN, LOW);
      long duration = pulseIn(ECHO_PIN, HIGH);
      distance = duration * 0.34/2;
      break;
    }
    //default case
    default:
    {
      Serial.println("ERROR: No valid sensortype");
    }   
  }

  //stop power to sensor
  if(ENABLE_PWR_PIN)
  {
    digitalWrite(PWR_PIN, HIGH);
  }
  
  //print output of sensor in cm
  Serial.print("********** Ultrasonic Distance: ");
  Serial.print(distance / 10);
  Serial.println(" cm"); 
}

///////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
#if(AT_SUPPORT)
  enableAt();
#endif
  deviceState = DEVICE_STATE_INIT;
  LoRaWAN.ifskipjoin();

  //activate pins
  if(ENABLE_PWR_PIN)
  {
    pinMode(PWR_PIN, OUTPUT);
  }
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop()
{
  switch( deviceState )
  {
    case DEVICE_STATE_INIT:
    {
#if(LORAWAN_DEVEUI_AUTO)
      LoRaWAN.generateDeveuiByChipID();
#endif
#if(AT_SUPPORT)
      getDevParam();
#endif
      printDevParam();
      LoRaWAN.init(loraWanClass,loraWanRegion);
      deviceState = DEVICE_STATE_JOIN;
      break;
    }
    case DEVICE_STATE_JOIN:
    {
      LoRaWAN.join();
      break;
    }
    case DEVICE_STATE_SEND:
    {
      //Read sensor
      getDistance();
      prepareTxFrame( appPort );
      LoRaWAN.send();
      deviceState = DEVICE_STATE_CYCLE;
      break;
    }
    case DEVICE_STATE_CYCLE:
    {
      // Schedule next packet transmission
      txDutyCycleTime = appTxDutyCycle + randr( 0, APP_TX_DUTYCYCLE_RND );
      LoRaWAN.cycle(txDutyCycleTime);
      deviceState = DEVICE_STATE_SLEEP;
      break;
    }
    case DEVICE_STATE_SLEEP:
    {
      LoRaWAN.sleep();
      break;
    }
    default:
    {
      deviceState = DEVICE_STATE_INIT;
      break;
    }
  }
}
