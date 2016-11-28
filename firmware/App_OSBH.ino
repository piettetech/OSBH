/**************************************************************************/
/*
    Open Source Beehives Sensor Kit Alpha v0.2

    Written by Scott Piette (Piette Technologies) October 4, 2014
    Developed for the Open Source Beehives Project
         (http://www.opensourcebeehives.net)
    Copyright (c) 2014 Open Source Beehives Project

    Beehive monitor using DHT and DS sensors
    Written for Spark Core (www.spark.io)
    
    Depends on the following libraries:
    - Modified Adafruit Base Class Sensor Library: https://github.com/piettetech/PietteTech_Sensor
      > Original Adafruit Unified Sensor Library: https://github.com/adafruit/Adafruit_Sensor
    - DHT Sensor Library: https://github.com/piettetech/PietteTech_DHT
      > Original idDHT Sensor Library: https://github.com/niesteszeck
    - DSX_U Unified Sensor Library: https://github.com/piettetech/PietteTech_DSX_U
      > Original Adafruit DHT Unified Sensor Library: https://github.com/adafruit/Adafruit_DHT_Unified
    - DHT_U Unified Sensor Library: https://github.com/piettetech/PietteTech_DHT_U
      > Original Adafruit DHT Unified Sensor Library: https://github.com/adafruit/Adafruit_DHT_Unified
    - Phant Library: https://github.com/piettetech/PietteTech_Phant
      > Original SparkFun Phant Library: https://github.com/sparkfun/phant-arduino
    - OneWire Library: https://github.com/Hotaman/OneWireSpark/fork
*/
/**************************************************************************/

#include "application.h"
#include "math.h"
#include "PietteTech_Phant.h"
#include "PietteTech_DSX_U.h"
#include "PietteTech_DHT_U.h"
#include "PietteTech_DHT.h"
#include "audio.h"
#include "weighing.h"
#include "sd-card-library.h"


/**************************************************************************/
/*!
        Program configuration options
*/
/**************************************************************************/
//#define CLEAR_STREAM_ON_START
#define OSBH_RECONNECT_LIMIT    10       // # times to try sending data
#define OSBH_REPORT_FREQUENCY 10 * 60	 // Report data every ## seconds

//disable wifi at Spark Startup
SYSTEM_MODE(SEMI_AUTOMATIC);

/**************************************************************************/
/*!
        Program debug options
*/
/**************************************************************************/
#define WAIT_FOR_KEYPRESS		 // wait for keypress in setup
#define SERIAL_DEBUG  2		         // 1 = timing, 2 = sensor , uncomment for no debugging
#if defined(SERIAL_DEBUG)
#define D(x) x						// build debug wrapper function
#else
#define D(x)
#endif
#if (SERIAL_DEBUG > 1)
#define DD2(x) x
#else
#define DD2(x)
#endif

/**************************************************************************/
/*!
        Loop Timing Debug Variables
*/
/**************************************************************************/
int _loopErrorCount;	        // # times (_loopReEntryTime * 2) is exceeded
int _loopCycleCount;	        // # times we have entered the loop
unsigned long _loopReEntryTime;	// average time to re-enter loop (first 50 samples)
unsigned long _exitLoopTime;	// time we exit our loop

/**************************************************************************/
/*!
        DHT Device Driver Initialization
*/
/**************************************************************************/
#define DHTPIN        4         // Pin for internal DHT sensor.
#define DHTTYPE       DHT22     // DHT 22 (AM2302)
#define DHTBPIN       5         // Pin for external DHT sensor.
#define DHTBTYPE      DHT22     // DHT 22 (AM2301)
void dht_wrapper();             // must be declared before object initialization Device 1
PietteTech_DHT dht(DHTPIN, DHTTYPE, dht_wrapper);
void dht_wrapper() { dht.isrCallback(); }// This wrapper calls isr
void dht_wrapperB();            // must be declared before object initialization Device 2
PietteTech_DHT dhtB(DHTBPIN, DHTBTYPE, dht_wrapperB);
void dht_wrapperB() { dhtB.isrCallback(); }// This wrapper calls isr

/**************************************************************************/
/*!
        DHT_U Driver Initialization
        NOTE:  Each DHT sensor has Temperature and Humidity
               We create two DHT_U objects, one for Temperature
               and one for Humidity.
*/
/**************************************************************************/
PietteTech_DHT_U _dht_u[4];          // Inside & Outside Temperature & Humidity

/**************************************************************************/
/*!
        DS Series OneWire devices
*/
/**************************************************************************/
#define MAX_DSX_DEVICES  2           // Max # devices we can scan for
#define ONEWIREPIN   3               // Pin connected to sensors.
OneWire one(ONEWIREPIN);             // - 4.7K pull-up resistor to 3.3v necessary
uint8_t ds_addr[8*MAX_DSX_DEVICES];  // Rom addresses of DSXXXX sensors
byte ds_num;                         // # sensors found

/**************************************************************************/
/*!
        DSX_U Driver
*/
/**************************************************************************/
PietteTech_DSX_U _dsx[MAX_DSX_DEVICES];   // Two remote beehive sensors

/**************************************************************************/
/*!
        Function:  scanOneWire
        Scan OneWire bus to find all DS sensors
        scans up to MAX_DEVICES sensors
*/
/**************************************************************************/
void scanOneWire()
{
    /*
     * Scan the one-wire bus for all devices (collect up to MAX_DEVICES)
     */
    uint8_t *r = &ds_addr[0];
    ds_num = 0;

    while (ds_num < MAX_DSX_DEVICES && one.search(r)) {
        if (OneWire::crc8(r, 7) == r[7]) {
            // the first ROM byte indicates which chip
            if (*r == DS18S20 || *r == DS18B20 || *r == DS1822 || r[0] == DS2438) {
                r+=8;
                ds_num++;
            }
        }
    }
    one.reset_search();
}

/**************************************************************************/
/*!
    Unified Sensor Object Array variables and defines
*/
/**************************************************************************/
#define MAX_SENSORS 6	                 // 2 1-Wire + 2 DHT @ 2x = 6 sensors
PietteTech_Sensor *_sensor[MAX_SENSORS]; // pointers to sensor objects
uint8_t _num_sensors;	                 // How many did we create
int sensorId;		                 // System unique sensor Id
uint32_t delayMS;	                 // Max delay for all connected sensors
unsigned long next_sensor_sample_time;	 // next time to read sensors

/**************************************************************************/
/*!
    Audio Sensor Initialisation
*/
/**************************************************************************/

int MICROPHONE = 10 ; // A0 on spark Core
int FFT_SIZE = 128 ;//FFT Bucket Size (32,64,128,256 - higher means more frequency resolution)
int SAMPLEDELAY = 600 ;//Delay for sampling in microseconds f = 1/t*10^6



/**************************************************************************/
/*!
    Weighning Sensor Initialisation
*/
/**************************************************************************/
// Max size of the buffer to calculate the average
// define stackMax 12		size of the average stack

// Pin out defs
int ADSK = D7;
int ADDO = D6;
//int TARE = D4;		// Tare pin not implemented yet

// init defaults for scale. Set via cal() function or manually by evaluating sensor data
int offset = 8400988;
int scale = 133/3;
int refWeight = 141;

/**************************************************************************/
/*!
    SD-Card Initialisation
*/
/**************************************************************************/

File myFile;

bool SD_attached = true; // define if you have a SD attached or not

// SOFTWARE SPI pin configuration - modify as required
// The default pins are the same as HARDWARE SPI
const uint8_t chipSelect = A2;    // Also used for HARDWARE SPI setup
const uint8_t mosiPin = A5;
const uint8_t misoPin = A4;
const uint8_t clockPin = A3;

/**************************************************************************/
/*!
    OSBH global variables and defines
*/
/**************************************************************************/
#define ONE_DAY_MILLIS (24 * 60 * 60 * 1000)
unsigned long lastSync = millis();	// last time we sync'd time

/**************************************************************************/
/*!
    Phant database objects and variables
    You will need to create your own data stream on the phant server
    and place the proper values into the creation line below

    <<host>>  This is the host for your phant data stream
	      examples =
			    data.osbh.smartcitizen.me
			    data.sparkfun.com

    <<Public Key>>  Public key provided when you create the stream
    <<Private Key>> Private key provided when you create the stream

*/
/**************************************************************************/
//Phant::Stream stream1("<<host>>", "<<Public Key>>", "<<Private Key>>");
Phant::Stream stream1("data.osbh.smartcitizen.me", "OGw6o7Z7AWsWVEZlEjj6", "8b2zAJPJrNFoB2VE2qqZ");
int _ret;                                // Return error code
D(int _s;)                               // Count of stream send attempts
unsigned long next_db_update_time;	 // next time to update phant db


/**************************************************************************/
/*
        Function:  SerialCurrentTime
        This fixes a bug in Spark Time where there is a line feed
        on the end of the string.
 */
/**************************************************************************/
void SerialCurrentTime() {
    D(char buf [50];)
    D(char *_c;)
    D(sprintf(buf, "[%s", Time.timeStr().c_str());)
    D(for(_c = buf; ((*_c != '\r') && (*_c != '\n') && (*_c != 0)); _c++) ;)
    D(*_c++ = ']';)
    D(*_c = 0;)
    D(Serial.print(buf);)
}


/**************************************************************************/
/*
        Function:  setup
 */
/**************************************************************************/
void setup() {
    int n;


    Serial.begin(9600);
#if defined(WAIT_FOR_KEYPRESS) // this is included for debugging mode. Entering a letter is not the usual way, but otherwise wouldn't work for all linux serial communication
    char check = 'o';
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
	// reserve memory for FFT (audio analysis)
	FFTinit();
	
  // configure pinsto be an input or output for weighning sensor
    pinMode(ADDO, INPUT);
    pinMode(ADSK, OUTPUT);
    //pinMode(TARE, INPUT);
    resetAD();

/**************************************************************************/
    // Initialize device.
    Serial.println("\n\rOpen Source Beehives Sensor Kit Alpha v1.0");
    Serial.print("\n\r");

	if(SD_attached) {
    // check SD-Card support
      Serial.print("Initializing SD card...");
   
		// Initialize HARDWARE SPI with user defined chipSelect
			if (!SD.begin(chipSelect)) {
			Serial.println("initialization failed!");
			SD_attached = false;
			return;
		}

			//  Comment out above lines and uncomment following lines to use SOFTWARE SPI
			/*
			  // Initialize SOFTWARE SPI
			  if (!SD.begin(mosiPin, misoPin, clockPin, chipSelect)) {
				Serial.println("initialization failed!");
				return;
			  }
			*/

		Serial.println("initialization done.");
	}

    // Scan the OneWire bus for devices
    Serial.print("Scanning for DSX devices - ");
    scanOneWire();
    Serial.print("found: "); Serial.print(ds_num); Serial.println(" sensors.");

    // Setup Sensor Objects for each one-wire device found
    char xname[12];
    strcpy(xname, "tempc_hive0");
    for (uint8_t *_addr = &ds_addr[0], n=0; n < ds_num; n++, _addr += 8) {
        xname[10] = n + '1';
        _dsx[n].begin(_addr, &one, ++sensorId, xname);
        _sensor[_num_sensors++] = &_dsx[n];
    }

    Serial.println("Setting up DHT22 inside & outside sensors.");
    
    // Setup DHT objects for each DHT sensor
    _dht_u[0].begin(&dht, DHTTYPE,  ++sensorId, SENSOR_TYPE_AMBIENT_TEMPERATURE, "tempc_core");
    _dht_u[1].begin(&dht, DHTTYPE,  sensorId, SENSOR_TYPE_RELATIVE_HUMIDITY, "hum_core");
    _dht_u[2].begin(&dhtB, DHTBTYPE,  ++sensorId, SENSOR_TYPE_AMBIENT_TEMPERATURE, "tempc");
    _dht_u[3].begin(&dhtB, DHTBTYPE,  sensorId, SENSOR_TYPE_RELATIVE_HUMIDITY, "hum");
    for (n = 0; n < 4; n++)
        _sensor[_num_sensors++] = &_dht_u[n];

    // Print temperature sensor details.
    sensor_t sensor;
    for(n = 0; n < _num_sensors; n++) {
        _sensor[n]->getSensor(&sensor);
        DD2(_sensor[n]->printSensorDetail(&sensor);)
        // Set delay between sensor readings based on sensor details.
        delayMS = sensor.min_delay / 1000;
    }

    
    
    
	// only if we are online
	if(Spark.connected()) {
    // Setup the Phant interface
    stream1.begin();
    // Lets sync up our time
    Spark.syncTime();
    lastSync = millis();
	}
	unsigned long _ts = millis();

	#if defined(CLEAR_STREAM_ON_START)
		//clearing previous stream values
		while(!(_ret = stream1.clearStream())) {
			Serial.println("Stream could not be cleared (connection failed)");
			delay(5000);
			_ts = millis();
		}
		Serial.println("Stream successfully cleared");
		Serial.print("Time to clear stream = ");
		float _f = (millis() - _ts) / 1000;
		Serial.print(_f, 2);
		Serial.println("s");
	#endif      // CLEAR_STREAM_ON_START
		// Setup the time for the next sensor read and database update
		next_sensor_sample_time = 1000 + delayMS; //changed from millis() to 1000
		next_db_update_time = next_sensor_sample_time;
	}

/**************************************************************************/
/*
        Function:  loop
 */
/**************************************************************************/
void loop() {

    unsigned long curTime = millis();
    unsigned long _deltaTime = curTime - _exitLoopTime;

    // This next section is for debugging the Spark
    // We measure the time it takes for the Spark to return control to
    // the loop.  We calculate the average for 1000 iterations and then
    // report if the average is ever exceeded.

/*#if 1
    // Reset the re-entry time to 1s every rollover of cycle count
    if (_loopCycleCount == 0)
	_loopReEntryTime = 1000;
    else if (_loopCycleCount < 100)
	_loopReEntryTime += _deltaTime;

    // Lets time how long it takes to re-enter the loop
    // and register an error if it is exceeded by 2x
    if ((_loopCycleCount > 100) && (_deltaTime > (_loopReEntryTime << 1))) {
	SerialCurrentTime();
	Serial.print(" - loop ReEntry exceeded [");
	Serial.print(++_loopErrorCount);
	Serial.print("] t = ");
	Serial.print(_deltaTime);
	Serial.println("ms");
    }

    // Lets average the first 100 cycles to determine the re-entry time
    if (++_loopCycleCount == 100) {
	_loopReEntryTime /= 100;
	SerialCurrentTime();
	Serial.print(" - loop ReEntry Cycle Time = ");
	Serial.print(_loopReEntryTime);
	Serial.println("ms");
    }
    // end of Spark loop timing debug section
#endif*/

    // Lets sync with the network time once a day
    if (curTime - lastSync > ONE_DAY_MILLIS) {
        D(SerialCurrentTime();)
        D(Serial.print(" - spark cloud time sync.\r\n");)
        Spark.syncTime();
        lastSync = millis();
    }

    if (curTime > next_sensor_sample_time) {
        // Get the sensor data and print its value.
        _num_sensors = 3;
        sensors_event_t event;
        DD2(sensor_t sensor;)
        for(int n = 0; n < _num_sensors; n++) {
            _sensor[n]->getEvent(&event);
            DD2(_sensor[n]->getSensor(&sensor);)
            DD2(_sensor[n]->printSensorEvent(&event, sensor.name);)
        }
        //print weight value
        DD2(Serial.print("Value in g:     ");)
        DD2(Serial.println( ADRead());)
        //perform audio analysis
        DD2(updateFFT();)
        

        
        // save data to SD CARD
        myFile = SD.open("OSBH.txt", FILE_WRITE);
			// if the file opened okay, write to it:
			  if (myFile) {
				char dataString[100] = "";
				Serial.print("savin to OSBH.txt...");
				//dataString += String(sensor);
				sprintf(dataString, "weight: %d", ADRead() );
				myFile.println(dataString); 
				for(int n = 0; n < _num_sensors; n++) {
				_sensor[n]->getEvent(&event);
				myFile.println(event.data[0]);
				}
			  // close the file:
				myFile.close();
				Serial.println("done.");
			  } else {
				// if the file didn't open, print an error:
				Serial.println("error opening OSBH.txt");
			  }
			    // re-open the file for reading:
				  myFile = SD.open("OSBH.txt");
				  if (myFile) {
					Serial.println("OSBH.txt:");
			     // read from the file until there's nothing else in it:
					while (myFile.available()) {
					  Serial.write(myFile.read());
					}
					// close the file:
					myFile.close();
				}
				Delay(5000);
		
        next_sensor_sample_time = curTime + delayMS;
    }
    
    

    // check if we need to send the sensor data to database
    if (curTime > next_db_update_time && Spark.connected()) {
	// Get sensor data and send to Phant
        sensor_t sensor;
        sensors_event_t event;
        for(int n = 0; n < _num_sensors; n++) {
            _sensor[n]->getSensor(&sensor);
            _sensor[n]->getEvent(&event);
            stream1.add(sensor.name, event.data[0]);
        }

        unsigned long _ts = millis();

        // Send data to Phant database
        int n;
        int _ret;
        for (n = 0; n < OSBH_RECONNECT_LIMIT; n++) {
            if (_ret = stream1.sendData()) break;
            delay(500);
        }

        // If we were unsuccessful sending the data clear the buffer
        if (!_ret) stream1.begin();

        // Lets print the time it took to send to the database
        D(SerialCurrentTime();)
        D(Serial.print(" - sample ["); Serial.print(++_s);)
        D(Serial.print("] Time to send stream = ");)
        D(float _f = (millis() - _ts) / 1000;)
        D(Serial.print(_f, 2);)
        D(if (n) {)
            D(Serial.print("s, [");)
            D(Serial.print(n);)
            D(Serial.println("] retrys.");)
        D(})
        D(else)
            D(Serial.println("s");)

        next_db_update_time = curTime + (OSBH_REPORT_FREQUENCY * 1000L);
    }
    _exitLoopTime = millis();	// save the time we exit our loop
	}
