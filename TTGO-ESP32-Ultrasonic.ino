#include <HardwareSerial.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <CayenneLPP.h>

// COPY the config.h.template and RENAME to config.h file in the same folder WITH YOUR TTN KEYS AND ADDR.
#include "config.h"

//DEFINE PINS
// PINS for trig and echo (used by sensortype TRIG_ECHO)
#define TRIG_PIN    13    // pin TRIG 
#define ECHO_PIN    12    // pin ECHO 
// PINS for RX and TX (UART Serial) (used by sensortype A02YYUW)
#define RX_PIN      17    // pin RX
#define TX_PIN      16    // pin TX


//Define Serial for UART connection
HardwareSerial mySerial(2);
unsigned char data[4]={};

//sensordata
float distance;


CayenneLPP lpp(51); // here we will construct Cayenne Low Power Payload (LPP) - see https://community.mydevices.com/t/cayenne-lpp-2-0/7510

// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}
void os_getDevKey (u1_t* buf) { memcpy_P(buf, APPKEY, 16);}

static osjob_t sendjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 30;

//Pin mapping for TTGO ESP32
const lmic_pinmap lmic_pins = {
    .nss = 18, 
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 14,
    .dio = {/*dio0*/ 26, /*dio1*/ 33, /*dio2*/ 32}
};


/*
 * Function to read distance from different sensortypes (float distance saves distance in mm)
 */
void getDistance() {
  switch (sensor)
  {
    //read sensor over UART (sensortype A0YYUW)
    case A0YYUW:
    {
      while (mySerial.available() > 0) {
        for (int i=0;i<4;i++) {
          data[i] = mySerial.read();
        }
      }
    
      mySerial.flush();
  
      if(data[0]==0xff)
      {
        int sum;
        sum=(data[0]+data[1]+data[2])&0x00FF;
        if(sum==data[3])
        {
          distance=(data[1]<<8)+data[2];
        }else Serial.println("ERROR");
      }
      else Serial.println("ERROR: first byte wrong");
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
      break;
    }
    //default case
    default:
    {
      Serial.println("ERROR: No valid sensortype");
    }   
  }

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
            // Schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
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


void setup() {
  Serial.begin(115200);
  mySerial.begin(9600, SERIAL_8N1, 16,17);

  // LMIC init
  os_init();
  //Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();
  

  // Start job
  do_send(&sendjob);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  

}

void loop() {
  os_runloop_once();
}
