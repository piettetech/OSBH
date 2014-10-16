/**************************************************************************/
/*!
    Open Source Bee Hive Sensor Kit v1.0

    Written by Scott Piette (Piette Technologies) October 4, 2014
    For Open Source Bee Hive Project

    Bee Hive monitor using DHT and DS sensors
    Written for Spark Core (www.spark.io)

    This work is licensed under a 
        Creative Commons Attribution-Share-Alike 4.0 License CC-BY-SA
        http://creativecommons.org/licenses/by-sa/4.0/
    
    Depends on the following libraries:
    - Modified Adafruit Base Class Sensor Library: https://github.com/mtnscott/PietteTech_Sensor
      > Original Adafruit Unified Sensor Library: https://github.com/adafruit/Adafruit_Sensor
    - DHT Sensor Library: https://github.com/mtnscott/PietteTech_DHT
      > Original idDHT Sensor Library: https://github.com/niesteszeck
    - DSX_U Unified Sensor Library: https://github.com/mtnscott/PietteTech_DSX_U
      > Original Adafruit DHT Unified Sensor Library: https://github.com/adafruit/Adafruit_DHT_Unified
    - DHT_U Unified Sensor Library: https://github.com/mtnscott/PietteTech_DHT_U
      > Original Adafruit DHT Unified Sensor Library: https://github.com/adafruit/Adafruit_DHT_Unified
    - Phant Library: https://github.com/mtnscott/PietteTech_Phant
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

/**************************************************************************/
/*!
        Program configuration options
*/
/**************************************************************************/
//#define CLEAR_STREAM_ON_START
#define OSBH_RECONNECT_LIMIT    6       // # times to try sending data
#define OSBH_REPORT_FREQUENCY 10 * 60	// Report data every ## seconds
//#define WAIT_FOR_KEYPRESS
#define SERIAL_DEBUG  1		// 1 = timing, 2 = sensor

#if defined(SERIAL_DEBUG)
#define D(x) x
#else
#define D(x)
#endif
#if (SERIAL_DEBUG > 1)
#define D2(x) x
#else
#define D2(x)
#endif

/**************************************************************************/
/*!
        DHT Device Driver Initialization
*/
/**************************************************************************/
#define DHTPIN        3         // Pin for internal DHT sensor.
#define DHTTYPE       DHT22     // DHT 22 (AM2302)
#define DHTBPIN       4         // Pin for external DHT sensor.
#define DHTBTYPE      DHT22     // DHT 22 (AM2301)
void dht_wrapper(); // must be declared before object initialization
PietteTech_DHT dht(DHTPIN, DHTTYPE, dht_wrapper);
void dht_wrapper() { dht.isrCallback(); }// This wrapper calls isr
void dht_wrapperB(); // must be declared before object initialization
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
PietteTech_DHT_U _dht_u[4];        // Inside & Outside Temperature & Humidity

/**************************************************************************/
/*!
        DS Series OneWire devices
*/
/**************************************************************************/
#define MAX_DSX_DEVICES  2           // Max # devices we can scan for
#define ONEWIREPIN   2           // Pin connected to sensors.
OneWire one(ONEWIREPIN);         // - 4.7K pull-up resistor to 3.3v necessary
uint8_t ds_addr[8*MAX_DSX_DEVICES];  // Rom addresses of DSXXXX sensors
byte ds_num;                     // # sensors found

/**************************************************************************/
/*!
        DSX_U Driver
*/
/**************************************************************************/
PietteTech_DSX_U _dsx[MAX_DSX_DEVICES];   // Two remote Hive sensors

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
#define MAX_SENSORS 6	// 2 1-Wire + 2 DHT @ 2x = 6 sensors
PietteTech_Sensor *_sensor[MAX_SENSORS]; // pointers to sensor objects
uint8_t _num_sensors;	// How many did we create
int sensorId;		// System unique sensor Id

uint32_t delayMS;	// Max delay for all connected sensors
unsigned long next_db_update_time;	// next time to update phant db
unsigned long next_sensor_sample_time;	// next time to read sensors
#define ONE_DAY_MILLIS (24 * 60 * 60 * 1000)
unsigned long lastSync = millis();	// last time we sync'd time

/**************************************************************************/
/*!
    Phant database objects and variables
*/
/**************************************************************************/
//Phant::Stream stream1("data.sparkfun.com", "0lz6AZVwJySgrrEEJw96", "D6nV1movXyHpnndd7PWr");
Phant::Stream stream1("data.osbh.smartcitizen.me", "2gVw0WQ0LbCVEKyvwKWLs4wyv14", "4oK4wYnwdBhRZJmEMJWYizZ043z");
int _ret;       // Return error code
D(int _s;)      // Count of stream send attempts


/**************************************************************************/
/*
        Function:  setup
 */
/**************************************************************************/
void setup() {
    int n;

    D(Serial.begin(9600);)
#if defined(WAIT_FOR_KEYPRESS)
    D(while(!Serial.available()) {)
        D(Serial.println("Press any key to begin");)
        D(delay(1000);)
    D(})
#endif

    // Initialize device.
    D(Serial.println("\n\rOpen Source Bee Hive Monitor v0.1");)
    D(Serial.println("Written by Scott Piette (Piette Technologies), October 2014");)
    D(Serial.print("\n\r");)

    // Scan the OneWire bus for devices
    D(Serial.print("Scanning for DSX devices - ");)
    scanOneWire();
    D(Serial.print("found: "); Serial.print(ds_num); Serial.println(" sensors.");)

    // Setup Sensor Objects for each one-wire device found
    char xname[12];
    strcpy(xname, "tempc_hive0");
    for (uint8_t *_addr = &ds_addr[0], n=0; n < ds_num; n++, _addr += 8) {
        xname[10] = n + '1';
        _dsx[n].begin(_addr, &one, ++sensorId, xname);
        _sensor[_num_sensors++] = &_dsx[n];
    }

    D(Serial.println("Setting up DHT22 inside & outside sensors.");)
    
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
        D2(_sensor[n]->printSensorDetail(&sensor);)
        // Set delay between sensor readings based on sensor details.
        delayMS = sensor.min_delay / 1000;
    }

    // Lets sync up our time
    Spark.syncTime();
    lastSync = millis();
    
    unsigned long _ts = millis();
    // Setup the Phant interface
    stream1.begin();

#if defined(CLEAR_STREAM_ON_START)
    //clearing previous stream values
    while(!(_ret = stream1.clearStream())) {
    	D(Serial.println("Stream could not be cleared (connection failed)");)
        delay(5000);
        _ts = millis();
    }
    D(Serial.println("Stream successfully cleared");)
    D(Serial.print("Time to clear stream = ");)
    D(float _f = (millis() - _ts) / 1000;)
    D(Serial.print(_f, 2);)
    D(Serial.println("s");)
#endif      // CLEAR_STREAM_ON_START

    // Setup the time for the next sensor read and database update
    next_sensor_sample_time = millis() + delayMS;
    next_db_update_time = next_sensor_sample_time;
}

/**************************************************************************/
/*
        Function:  loop
 */
/**************************************************************************/
void loop() {

    unsigned long curTime = millis();

    // Lets sync with the network time once a day
    if (curTime - lastSync > ONE_DAY_MILLIS) {
        D(Serial.print("Sync Time.\n");)
        Spark.syncTime();
        lastSync = millis();
    }

    if (curTime > next_sensor_sample_time) {
        // Get the sensor data and print its value.
        sensors_event_t event;
        D2(sensor_t sensor;)
        for(int n = 0; n < _num_sensors; n++) {
            _sensor[n]->getEvent(&event);
            D2(_sensor[n]->getSensor(&sensor);)
            D2(_sensor[n]->printSensorEvent(&event, sensor.name);)
        }
        next_sensor_sample_time = curTime + delayMS;
    }

    // check if we need to send to the database
    if (curTime > next_db_update_time) {
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
        for (n = 0; n < OSBH_RECONNECT_LIMIT; n++)
            if (stream1.sendData()) break;

        D(Serial.println(Time.timeStr());)
        D(Serial.print("Sample ["); Serial.print(++_s);)
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
    
}
