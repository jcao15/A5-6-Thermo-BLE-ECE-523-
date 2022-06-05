#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

#include <Adafruit_CircuitPlayground.h>

#if SOFTWARE_SERIAL_AVAILABLE
  #include <SoftwareSerial.h>
#endif


Adafruit_BluefruitLE_UART ble(BLUEFRUIT_HWSERIAL_NAME, BLUEFRUIT_UART_MODE_PIN);


void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

/* The service information */

int32_t tempBtnServiceId;
int32_t tempCharId;
int32_t btnCharId;
/**************************************************************************/

int tempC, tempF;

bool leftButtonPressed, rightButtonPressed, ButtonPressed;
/*!
    @brief  Sets up the HW an the BLE module (this function is called
            automatically on startup)
*/
/**************************************************************************/
void setup(void)
{
  while (!Serial); // required for Flora & Micro
  delay(500);

  boolean success;

  Serial.begin(115200);
  CircuitPlayground.begin();
  Serial.println(F("Adafruit Temperature and Button"));
  Serial.println(F("-------------------------------"));

  randomSeed(micros());

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  /* Perform a factory reset to make sure everything is in a known state */
  Serial.println(F("Performing a factory reset: "));
  if (! ble.factoryReset() ){
       error(F("Couldn't factory reset"));
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  // this line is particularly required for Flora, but is a good idea
  // anyways for the super long lines ahead!
  ble.setInterCharWriteDelay(5); // 5 ms

  /* Change the device name to make it easier to find */
  Serial.println(F("Setting device name to 'UW Thermo-Clicker': "));

  if (! ble.sendCommandCheckOK(F("AT+GAPDEVNAME=UW Thermo-Clicker")) ) {
    error(F("Could not set device name?"));
  }

  /* Add the Heart Rate Service definition */
  /* Service ID should be 1 */
  Serial.println(F("Adding the temperature button Service definition (UUID = 0x180D): "));
  success = ble.sendCommandWithIntReply( F("AT+GATTADDSERVICE=UUID=0x180D"), &tempBtnServiceId);
  if (! success) {
    error(F("Could not add TEMP service"));
  }


  /* Chars ID for Measurement should be 1 */
  
  Serial.println(F("Adding the Temperature Measurement characteristic (UUID = 0x2A37): "));
  success = ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A37, PROPERTIES=0x10, MIN_LEN=2, MAX_LEN=3, VALUE=00-40"), &tempCharId);
    if (! success) {
    error(F("Could not add Temperature characteristic"));
  }


  /* Chars ID for Body should be 2 */
  Serial.println(F("Adding the Button characteristic (UUID = 0x2A38): "));
  success = ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A38, PROPERTIES=0x10, MIN_LEN=2, MAX_LEN=3, VALUE=00-40"), &btnCharId);
    if (! success) {
    error(F("Could not add Button characteristic"));
  }

  /* Add the Temperature readding Service to the advertising data  */
  Serial.print(F("Adding Temperature Button Service UUID to the advertising payload: "));
  ble.sendCommandCheckOK( F("AT+GAPSETADVDATA=02-01-06-05-02-0d-18-0a-18") );

  /* Reset the device for the new service setting changes to take effect */
  Serial.print(F("Performing a SW reset (service changes require a reset): "));
  ble.reset();

  Serial.println();
}

/** Send randomized heart rate data continuously **/
void loop(void)
{
  tempC = CircuitPlayground.temperature();
  tempF = CircuitPlayground.temperatureF();

  leftButtonPressed = CircuitPlayground.leftButton();
  rightButtonPressed = CircuitPlayground.rightButton();
  ButtonPressed = leftButtonPressed || rightButtonPressed;

  /* Temperature */
  
  Serial.print("tempC: ");
  Serial.print(tempC);
  Serial.print("  tempF: ");
  Serial.println(tempF);

  /* Command is sent when \n (\r) or println is called */
  /* AT+GATTCHAR=CharacteristicID,value */
  
  ble.print( F("AT+GATTCHAR=") );
  ble.print( tempCharId );
  ble.print( F(",00-") );
  ble.println(tempF, HEX);
  
  
  
  /* Button */
  Serial.print("Button State: ");
  if (ButtonPressed) {
    Serial.println("DOWN");
  } else {
    Serial.println("UP");
  }

  ble.print( F("AT+GATTCHAR=") );
  ble.print( btnCharId );
  ble.print( F(",00-") );
  ble.println(int(ButtonPressed), HEX);

  
  /* Check if command executed OK */
  if ( !ble.waitForOK() )
  {
    Serial.println(F("Failed to get response!"));
  }

  /* Delay before next measurement update */
  delay(1000);
}
