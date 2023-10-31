/*
 * 
 * 
 * Codesnippets:
 *   - ESP32 DeepSleep function: https://jackgruber.github.io/2020-04-13-ESP32-DeepSleep-and-LoraWAN-OTAA-join/
 */


#include <HardwareSerial.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <CayenneLPP.h>

// COPY the config.h.template and RENAME to config.h file in the same folder WITH YOUR TTN KEYS AND ADDR.
#include "config.h"

//Define Serial for UART connection
HardwareSerial mySerial(2);
unsigned char data[4]={};

//sensordata
float distance;

//Use RTC memory to store LMIC configurations
RTC_DATA_ATTR lmic_t RTC_LMIC;

//global var to check if we can go to deep sleep
bool GOTO_DEEPSLEEP = false;


CayenneLPP lpp(51); // here we will construct Cayenne Low Power Payload (LPP) - see https://community.mydevices.com/t/cayenne-lpp-2-0/7510

// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}
void os_getDevKey (u1_t* buf) { memcpy_P(buf, APPKEY, 16);}

static osjob_t sendjob;



/*
 * Function to read distance from different sensortypes (float distance saves distance in mm)
 */
void getDistance() {
  // start power if enabled
  if(ENABLE_PWR_PIN)
  {
    digitalWrite(PWR_PIN, HIGH);
    delay(1000);
  }
  
  switch (SENSOR)
  {
    //read sensor over UART (sensortype A0YYUW)
    case A0YYUW:
    {
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
      distance = duration * 0.343/2;
      Serial.println(duration);
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
    digitalWrite(PWR_PIN, LOW);
  }
  
  //print output of sensor in cm
  Serial.print("********** Ultrasonic Distance: ");
  Serial.print(distance / 10);
  Serial.println(" cm"); 
}


/*
 * Help function for LMIC
 */
void printHex2(unsigned v) {
    v &= 0xff;
    if (v < 16)
        Serial.print('0');
    Serial.print(v, HEX);
}

/*
 * Event function of LMIC library
 */
void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            {
              u4_t netid = 0;
              devaddr_t devaddr = 0;
              u1_t nwkKey[16];
              u1_t artKey[16];
              LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);
              Serial.print("netid: ");
              Serial.println(netid, DEC);
              Serial.print("devaddr: ");
              Serial.println(devaddr, HEX);
              Serial.print("AppSKey: ");
              for (size_t i=0; i<sizeof(artKey); ++i) {
                if (i != 0)
                  Serial.print("-");
                printHex2(artKey[i]);
              }
              Serial.println("");
              Serial.print("NwkSKey: ");
              for (size_t i=0; i<sizeof(nwkKey); ++i) {
                      if (i != 0)
                              Serial.print("-");
                      printHex2(nwkKey[i]);
              }
              Serial.println();
            }
            // Disable link check validation (automatically enabled
            // during join, but because slow data rates change max TX
            // size, we don't use it in this example.
            LMIC_setLinkCheckMode(0);
            break;
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
              Serial.print(F("Received "));
              Serial.print(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
            }
            GOTO_DEEPSLEEP = true;
            // Schedule next transmission
            // os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
        case EV_TXSTART:
            Serial.println(F("EV_TXSTART"));
            break;
        case EV_TXCANCELED:
            Serial.println(F("EV_TXCANCELED"));
            break;
        case EV_RXSTART:
            /* do not print anything -- it wrecks timing */
            break;
        case EV_JOIN_TXCOMPLETE:
            Serial.println(F("EV_JOIN_TXCOMPLETE: no JoinAccept"));
            break;

        default:
            Serial.print(F("Unknown event: "));
            Serial.println((unsigned) ev);
            break;
    }
}

/*
 * LMIC function which sends data
 */
void do_send(osjob_t* j){
  //get distance from sensor
  getDistance();
  delay(1000);
  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND) {
    Serial.println(F("OP_TXRXPEND, not sending"));
  } else {
    //construct LPP packet
    lpp.reset();
    lpp.addLuminosity(1, distance); //use Luminosity to send data with uint16
    // Prepare upstream data transmission at the next possible time.
    LMIC_setTxData2(1, lpp.getBuffer(), lpp.getSize(), 0);
    Serial.println(F("Packet queued"));
  }
  // Next TX is scheduled after TX_COMPLETE event.
}

/*
 * Save and Load LMIC data for deep sleep
 * https://jackgruber.github.io/2020-04-13-ESP32-DeepSleep-and-LoraWAN-OTAA-join/
 */
 void SaveLMICToRTC(int deepsleep_sec)
{
    RTC_LMIC = LMIC;
    // EU Like Bands

    //System time is resetted after sleep. So we need to calculate the dutycycle with a resetted system time
    unsigned long now = millis();
#if defined(CFG_LMIC_EU_like)
    for(int i = 0; i < MAX_BANDS; i++) {
        ostime_t correctedAvail = RTC_LMIC.bands[i].avail - ((now/1000.0 + deepsleep_sec ) * OSTICKS_PER_SEC);
        if(correctedAvail < 0) {
            correctedAvail = 0;
        }
        RTC_LMIC.bands[i].avail = correctedAvail;
    }

    RTC_LMIC.globalDutyAvail = RTC_LMIC.globalDutyAvail - ((now/1000.0 + deepsleep_sec ) * OSTICKS_PER_SEC);
    if(RTC_LMIC.globalDutyAvail < 0) 
    {
        RTC_LMIC.globalDutyAvail = 0;
    }
#else
    Serial.println("No DutyCycle recalculation function!")
#endif
}

void LoadLMICFromRTC()
{
    LMIC = RTC_LMIC;
}

/*
 * Function to start deep sleep of ESP 
 */
void GoDeepSleep()
{
  Serial.println(F("Go DeepSleep"));
  Serial.flush();
  esp_sleep_enable_timer_wakeup(TX_INTERVAL * 1000000);
  esp_deep_sleep_start();
}

/*
 * Setup fupnction. Will be runed after each deep sleep and at start
 */
void setup() {
  Serial.begin(115200);
  mySerial.begin(9600, SERIAL_8N1, 16,17);

  //activate pins
  if(ENABLE_PWR_PIN)
  {
    pinMode(PWR_PIN, OUTPUT);
  }
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // LMIC init
  os_init();
  //Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();

  // Load the LoRa information from RTC if available
  if(RTC_LMIC.seqnoUp != 0)
  { 
      LoadLMICFromRTC();
  }

  // Start job
  do_send(&sendjob);


  

}

/*
 * Loop is checking if we can go to deep sleep after sending data to TTN
 */
void loop() {
  static unsigned long lastPrintTime = 0;
  
  os_runloop_once();

  //Check if deep sleep is posibble or a time critical job is running
  const bool timeCriticalJobs = os_queryTimeCriticalJobs(ms2osticksRound((TX_INTERVAL * 1000)));
  if (!timeCriticalJobs && GOTO_DEEPSLEEP == true && !(LMIC.opmode & OP_TXRXPEND))
  {
    Serial.print(F("Can go sleep "));
    SaveLMICToRTC(TX_INTERVAL);
    GoDeepSleep();
  }
  else if (lastPrintTime + 2000 < millis())
  {
    Serial.print(F("Cannot sleep "));
    Serial.print(F("TimeCriticalJobs: "));
    Serial.print(timeCriticalJobs);
    Serial.print(" ");

    lastPrintTime = millis();
  }
}
