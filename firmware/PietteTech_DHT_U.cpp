/*
 * PietteTech_DHT_U
 *
 *  Adaption of Adafruit DHT Unified driver to Spark and DHT library
 *
 *  Adapted by Scott Piette (Piette Technologies, LTD)
 *  Copyright (c) 2014 Scott Piette (scott.piette@gmail.com)
 *  Developed for the Open Source Beehives Project
 *       (http://www.opensourcebeehives.net)

 *  This adaptation is released under the following license:
 *	GPL v3 (http://www.gnu.org/licenses/gpl.html)
 *
 *  October 3, 2014
 * 	Added support for using this object as Temperature or Humidity
 * 	Set senseType in setType method to define sensor from PietteTech_Sensor.h
 * 	Added support for naming each sensor object
 * 	Set name in setType method for identifying sensor
 *
 * 	Use same low level DHT driver for multiple objects
 * 	 - for example one DHT can have Humidity & Temperature
 *	This improves usability of Unified library
 *
 */

// DHT Temperature & Humidity Unified Sensor Library
// Copyright (c) 2014 Adafruit Industries
// Author: Tony DiCola

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "PietteTech_DHT_U.h"

/**************************************************************************/
/*
        Constructor
 */
/**************************************************************************/
PietteTech_DHT_U::PietteTech_DHT_U() {}

/**************************************************************************/
/*
        Sets parameters
 */
/**************************************************************************/
void PietteTech_DHT_U::begin(PietteTech_DHT* dht, uint8_t dhtType, int32_t sensorId, int32_t senseType, char *sensorName)
{
    _dht = dht;
    _dht_type = dhtType;
    _sensorID = sensorId;

    _sense_type = senseType;
    if (sensorName) {
        strncpy(_name, sensorName, sizeof(_name) - 1);
        _name[sizeof(_name)- 1] = 0;
    }
//   if (name)
//        setType(senseType, name);
}

/**************************************************************************/
/*
        Populates the sensor_t name for this sensor
 */
/**************************************************************************/
void PietteTech_DHT_U::setName(sensor_t* sensor) {
    if (_name) {
        strncpy(sensor->name, _name, sizeof(sensor->name) - 1);
    } else {
        switch(_dht_type) {
            case DHT11:
                strncpy(sensor->name, "DHT11", sizeof(sensor->name) - 1);
                break;
            case DHT21:
                strncpy(sensor->name, "DHT21", sizeof(sensor->name) - 1);
                break;
            case DHT22:
                strncpy(sensor->name, "DHT22", sizeof(sensor->name) - 1);
                break;
            default:
                // TODO: Perhaps this should be an error?  However main DHT library doesn't enforce
                // restrictions on the sensor type value.  Pick a generic name for now.
                strncpy(sensor->name, "DHT?", sizeof(sensor->name) - 1);
                break;
        }
    }
    sensor->name[sizeof(sensor->name)- 1] = 0;
}

/**************************************************************************/
/*!
        Provides the min, max, and delay sensor_t data
 */
/**************************************************************************/
void PietteTech_DHT_U::setMinMaxDelay(sensor_t* sensor) {
    sensor->type = _sense_type;
    if (_sense_type == SENSOR_TYPE_AMBIENT_TEMPERATURE)
    {
        switch (_dht_type) {
            case DHT11:
                sensor->max_value   = 50.0F;
                sensor->min_value   = 0.0F;
                sensor->resolution  = 1.0F;
                sensor->min_delay = 1000000L;  // 1 second (in microseconds)
                break;
            case DHT21:     // TODO
                sensor->max_value   = 100.0F;
                sensor->min_value   = 0.0F;
                sensor->resolution  = 0.1F;
                sensor->min_delay = 2000000L;  // 2 seconds (in microseconds)
                break;
            case DHT22:
                sensor->max_value   = 80.0F;
                sensor->min_value   = -40.0F;
                sensor->resolution  = 0.5F;
                sensor->min_delay = 2000000L;  // 2 seconds (in microseconds)
                break;
            default:        // TODO
                // Unknown type, default to 0.
                sensor->max_value   = 0.0F;
                sensor->min_value   = 0.0F;
                sensor->resolution  = 0.0F;
                // Default to slowest sample rate in case of unknown type.
                sensor->min_delay = 2000000L;  // 2 seconds (in microseconds)
                break;
        }
    } else {    // Humidity
        switch (_dht_type) {
            case DHT11:
                sensor->max_value   = 80.0F;
                sensor->min_value   = 20.0F;
                sensor->resolution  = 5.0F;
                sensor->min_delay = 1000000L;  // 1 second (in microseconds)
                break;
            case DHT21: // TODO
                sensor->max_value   = 99.0F;
                sensor->min_value   = 0.0F;
                sensor->resolution  = 2.0F;
                sensor->min_delay = 2000000L;  // 2 seconds (in microseconds)
                break;
            case DHT22:
                sensor->max_value   = 99.0F;
                sensor->min_value   = 0.0F;
                sensor->resolution  = 2.0F;
                sensor->min_delay = 2000000L;  // 2 seconds (in microseconds)
                break;
            default:    // TODO
                // Unknown type, default to 0.
                sensor->max_value   = 0.0F;
                sensor->min_value   = 0.0F;
                sensor->resolution  = 0.0F;
                // Default to slowest sample rate in case of unknown type.
                sensor->min_delay = 2000000L;  // 2 seconds (in microseconds)
                break;
        }
        
    }
}

/**************************************************************************/
/*!
        Provides the sensor_t data for this sensor
 */
/**************************************************************************/
void PietteTech_DHT_U::getSensor(sensor_t *sensor)
{
    /* Clear the sensor_t object */
    memset(sensor, 0, sizeof(sensor_t));
    // Set sensor name.
    setName(sensor);
    // Set version and ID
    sensor->version     = 1;
    sensor->sensor_id   = _sensorID;
    // Set type and characteristics.
    sensor->type = _sense_type;
    setMinMaxDelay(sensor);
}

/**************************************************************************/
/*!
        Reads the sensor and returns the data as a sensors_event_t
 */
/**************************************************************************/
void PietteTech_DHT_U::getEvent(sensors_event_t* event) {
    // Clear event definition.
    memset(event, 0, sizeof(sensors_event_t));
    // Populate sensor reading values.
    event->version     = sizeof(sensors_event_t);
    event->sensor_id   = _sensorID;
    event->type        = _sense_type;
    event->timestamp   = millis();
    if (_sense_type == SENSOR_TYPE_AMBIENT_TEMPERATURE)
        event->temperature = _dht->readTemperature();
    else
        event->relative_humidity = _dht->readHumidity();
}
