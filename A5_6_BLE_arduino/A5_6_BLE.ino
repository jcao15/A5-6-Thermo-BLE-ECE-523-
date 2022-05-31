/*********************************************************************
    Laura Arjona. UW EE P 523. SPRING 2020
    Example of simple interaction beteween Adafruit Circuit Playground
    and Android App. Communication with BLE - uart
*********************************************************************/

/********************************************************************
   Musical Notes via https://www.arduino.cc/en/Tutorial/ToneMelody
 ********************************************************************/

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978





#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#include <Adafruit_CircuitPlayground.h>

#include "BluefruitConfig.h"

#if SOFTWARE_SERIAL_AVAILABLE
#include <SoftwareSerial.h>
#endif


// Strings to compare incoming BLE messages
String start = "start";
String red = "red";
String readtemp = "readtemp";
String stp = "s";
String changeTone = "ct";
unsigned long curr;
unsigned long curr2;
int frequency = 500;
unsigned long duration = 100;
unsigned long timeup;
unsigned long oneminute=60000;
int  sensorTemp = 0;
float y;
int tone_number = 1;
bool snoozed = false;
bool out_true = false;
unsigned long ten = 10;
unsigned long snoozeTime = 600000;
unsigned long snoozetimeup;



const int numNotes = 8;                     // number of notes we are playing
//structure music:
struct Melody {
  int melody[8];
  int noteDurations[8];
} melodies[] = {
  {{ NOTE_C7, NOTE_G6, NOTE_G6, NOTE_A6, NOTE_G6, 0, NOTE_B6, NOTE_C7 }, {4, 8, 8, 4, 4, 4, 4, 4 } }, // tone1
  {{ NOTE_B6, NOTE_B6, NOTE_B6, NOTE_B6, NOTE_D7, NOTE_G6, NOTE_A6, NOTE_B6 }, { 4, 4, 2, 8, 8, 8, 8, 2 } },
  {{ NOTE_E7, NOTE_E7, NOTE_E7, NOTE_C7, NOTE_E7, NOTE_G7, 0, NOTE_G6}, { 8, 8, 8, 16, 16, 8, 8, 8} }
};


/*=========================================================================
    APPLICATION SETTINGS
    -----------------------------------------------------------------------*/
#define FACTORYRESET_ENABLE         0
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"
/*=========================================================================*/

// Create the bluefruit object, either software serial...uncomment these lines

Adafruit_BluefruitLE_UART ble(BLUEFRUIT_HWSERIAL_NAME, BLUEFRUIT_UART_MODE_PIN);

/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
// Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

/* ...software SPI, using SCK/MOSI/MISO user-defined SPI pins and then user selected CS/IRQ/RST */
//Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_SCK, BLUEFRUIT_SPI_MISO,
//                             BLUEFRUIT_SPI_MOSI, BLUEFRUIT_SPI_CS,
//                             BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);


// A small helper to show errors on the serial monitor
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}


void setup(void)
{
  CircuitPlayground.begin();
  
  CircuitPlayground.setAccelRange(LIS3DH_RANGE_2_G);

  Serial.begin(115200);

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ) {
      error(F("Couldn't factory reset"));
    }
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  Serial.println(F("Please use Adafruit Bluefruit LE app to connect in UART mode"));
  Serial.println(F("Then Enter characters to send to Bluefruit"));
  Serial.println();

  ble.verbose(false);  // debug info is a little annoying after this point!

  /* Wait for connection */
  while (! ble.isConnected()) {
    delay(500);
  }
  Serial.println("CONECTED:");
  Serial.println(F("******************************"));
  //Send data to Android Device
  delay(5000);
  char output[8];
  String data = "";
  data += "c";
  Serial.println(data);
  data.toCharArray(output, 8);
  ble.print(data);

  // LED Activity command is only supported from 0.6.6
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    // Change Mode LED Activity
    Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
  }

  // Set module to DATA mode
  Serial.println( F("Switching to DATA mode!") );
  ble.setMode(BLUEFRUIT_MODE_DATA);

  Serial.println(F("******************************"));

  CircuitPlayground.setPixelColor(20, 20, 20, 20);

  delay(1000);
}
/**************************************************************************/
/*!
   Constantly poll for new command or response data
*/
/**************************************************************************/
void loop(void)
{
out:
    //if(out_true == false){
    sensorTemp = CircuitPlayground.temperature();
    Serial.println("Read temperature sensor");
    delay(10);
    //Send data to Android Device
    char output[8];
    String data = "";
    data += sensorTemp;
    Serial.println(data);
    data.toCharArray(output, 8);
    ble.print(data);
  }


}
