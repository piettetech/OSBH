/*
  SD card datalogger
 
 This example shows how to log data from three analog sensors 
 to an SD card using the SD library.
  
 The circuit:
 * analog sensors on analog ins 0, 1, and 2
 * SD card attached to SPI bus as follows:
  Refer to "libraries/SdFat/Sd2Card_config.h" 
 
 created  24 Nov 2010
 updated 2 Dec 2010
 by Tom Igoe
 This example code is in the public domain.
 modified for Maple(STM32 micros)/libmaple
 17 Mar 2012
 by dinau  
 */

#include "application.h"
#include "sd-card-library.h"


//disable wifi at Spark Startup
SYSTEM_MODE(SEMI_AUTOMATIC);

#define WAIT_FOR_KEYPRESS // serielle Verbindung bei Setup abwarten

// SOFTWARE SPI pin configuration - modify as required
// The default pins are the same as HARDWARE SPI
const uint8_t chipSelect = A2;    // Also used for HARDWARE SPI setup
const uint8_t mosiPin = A5;
const uint8_t misoPin = A4;
const uint8_t clockPin = A3;

// change this to match your SD shield or module;

char dataString[100];
char sdata[10];
int lpcount=10;

// set up variables using the SD utility library functions:

void setup()
{
  char check = 'o';

    Serial.begin(9600);
#if defined(WAIT_FOR_KEYPRESS) // this is included for debugging mode. Entering a letter is not the usual way, but otherwise wouldn't work for all linux serial communication
    while(check != 'a') {
	delay(500);
	Serial.println(" enter <a> to begin or <w> to enable wifi"); // Entering a letter is not the usual way, but otherwise wouldn't work for all linux serial communications
	check = Serial.read();
	if(check == 'w') {
		WiFi.on();
		Spark.connect();
		if(Spark.connected()) {
			Serial.println("online!");
		}
	}
	Spark.process();  // keep the spark happy
	delay(500);
    }
#endif
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  
//  Comment out above lines and uncomment following lines to use SOFTWARE SPI
/*
  // Initialize SOFTWARE SPI
  if (!SD.begin(mosiPin, misoPin, clockPin, chipSelect)) {
    Serial.println("Card failed, or not present");
    return;
  }
*/

  Serial.println("card initialized.");
}

void loop()
{
  if( lpcount-- == 0 ){ while(1); }

  // make a string for assembling the data to log:
  //String dataString = "";
  dataString[0]='\0';
  // read three sensors and append to the string:
  for (int analogPin = 0; analogPin < 2; analogPin++) {
    int sensor = analogRead(analogPin);
    //dataString += String(sensor);
  sprintf(sdata, "%d", sensor );
  strcat( dataString, sdata);
    if (analogPin == 0) {
      //dataString += ",";
    strcat( dataString, ",");
    }
  }

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
  }  
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  } 
}
