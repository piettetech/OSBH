/*
 *  PietteTech_DSX_U
 *
 *  Adaption of Adafruit Unified driver to Spark and DS OneWire sensors
 *
 *  Adapted by Scott Piette (Piette Technologies, LTD)
 *  Copyright (c) 2014 Scott Piette (scott.piette@gmail.com)
 *  Developed for the Open Source Beehives Project
 *       (http://www.opensourcebeehives.net)
 *
 *  This adaptation is released under the following license:
 *	GPL v3 (http://www.gnu.org/licenses/gpl.html)
 *
 *  October 3, 2014
 * 	Added support for naming each sensor object
 * 	Set name in setType method for identifying sensor
 *
 * 	Use same low level OneWire driver for multiple objects
 * 	 - for example one pin can have multiple Temperature sensors
 *	This improves usability of Unified library
 *
 */

#include "PietteTech_DSX_U.h"

/**************************************************************************/
/*!
 @brief  Instantiates a new PietteTech_DSX_U class
 */
/**************************************************************************/
PietteTech_DSX_U::PietteTech_DSX_U() {}

/***************************************************************************
 PUBLIC FUNCTIONS
 ***************************************************************************/

/**************************************************************************/
/*!
 @brief  Setups the HW
 */
/**************************************************************************/
void PietteTech_DSX_U::begin(uint8_t *addr, OneWire *one, int32_t sensorId, char *sensorName)
{
    memcpy(&_addr[0], addr, 8);
    _one = one;
    _sensorID = sensorId;

    if (sensorName) {
        strncpy(_name, sensorName, sizeof(_name) - 1);
        _name[sizeof(_name)- 1] = 0;
    }
}

/**************************************************************************/
/*!
 @brief  Reads the temperatures in degrees Celsius
 */
/**************************************************************************/
float PietteTech_DSX_U::getTemperature()
{
    // If the sensor type is unknown return
    if (_addr[0] == -1)
        return (float) 0;
    
    uint8_t resp[9];      		// 9 byte response buffer
    byte i;
//    byte present = 0;
    
    _one->reset();              	// Issue reset to one-wire buss
    _one->select(&_addr[0]);        	// Send rom address
    
    _one->write(DSX_CMD_STARTCONVERSION);// start conversion, with parasite power on at the end
    delay(10);            // maybe 750ms is enough, maybe not, I'm shooting for 1 reading per second
    
    _one->reset(); 			// Issue reset to bus
    _one->select(&_addr[0]);        	// Send rom address
    _one->write(DSX_CMD_READSCRATCHPAD, 0);// Read Scratchpad 0
    
    for ( i = 0; i < 9; i++)            // we need 9 bytes
        resp[i] = _one->read();
    
    // Convert the data to actual temperature
    // because the result is a 16 bit signed integer, it should
    // be stored to an "int16_t" type, which is always 16 bits
    // even when compiled on a 32 bit processor.
    int16_t raw = (resp[1] << 8) | resp[0];
    if (_addr[0] == DS18S20) {    // DS18S20
        raw = raw << 3; // 9 bit resolution default
        if (resp[7] == 0x10) {
            // "count remain" gives full 12 bit resolution
            raw = (raw & 0xFFF0) + 12 - resp[6];
        }
        return (float)raw / 16.0;
    } else if (_addr[0] == DS2438) { // DS2438
        if (resp[2] > 127) resp[2]=0;
        resp[1] = resp[1] >> 3;
        return (float)resp[2] + ((float)resp[1] * .03125);
    }
    // DS18B20 and DS1822
    byte cfg = (resp[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
    return  (float)raw / 16.0;
}

/**************************************************************************/
/*!
 @brief  Populates the sensor_t name for this sensor
 */
/**************************************************************************/
void PietteTech_DSX_U::setName(sensor_t* sensor) {
    if (_name) {
        strncpy(sensor->name, _name, sizeof(sensor->name) - 1);
    } else {
        switch(_addr[0]) {
            case DS18S20:
                strncpy(sensor->name, "DS18S20", sizeof(sensor->name) - 1);
                break;
            case DS18B20:
                strncpy(sensor->name, "DS18B20", sizeof(sensor->name) - 1);
                break;
            case DS1822:
                strncpy(sensor->name, "DS1822", sizeof(sensor->name) - 1);
                break;
            case DS2438:
                strncpy(sensor->name, "DS2438", sizeof(sensor->name) - 1);
                break;
            default:
                // TODO: Perhaps this should be an error?  However main DHT library doesn't enforce
                // restrictions on the sensor type value.  Pick a generic name for now.
                strncpy(sensor->name, "DSX?", sizeof(sensor->name) - 1);
                break;
        }
    }
    sensor->name[sizeof(sensor->name)- 1] = 0;
}

/**************************************************************************/
/*!
 @brief  Populates the sensor_t name for this sensor
 */
/**************************************************************************/
void PietteTech_DSX_U::setMinDelay(sensor_t* sensor) {
    switch(_addr[0]) {
        case DS18S20:
            sensor->min_delay = 1000000L;  // 1 second (in microseconds)
            break;
        case DS18B20:
            sensor->min_delay = 2000000L;  // 2 seconds (in microseconds)
            break;
        case DS1822:
            sensor->min_delay = 2000000L;  // 2 seconds (in microseconds)
            break;
        case DS2438:
            sensor->min_delay = 2000000L;  // 2 seconds (in microseconds)
            break;
        default:
            // Default to slowest sample rate in case of unknown type.
            sensor->min_delay = 2000000L;  // 2 seconds (in microseconds)
            break;
    }
}

/**************************************************************************/
/*!
 @brief  Provides the sensor_t data for this sensor
 */
/**************************************************************************/
void PietteTech_DSX_U::getSensor(sensor_t *sensor)
{
    /* Clear the sensor_t object */
    memset(sensor, 0, sizeof(sensor_t));
    
    // Set sensor name.
    setName(sensor);
    // Set version and ID
    sensor->version     = 1;
    sensor->sensor_id   = _sensorID;
    // Set type and characteristics.
    sensor->type        = SENSOR_TYPE_AMBIENT_TEMPERATURE;
    setMinDelay(sensor);
    switch (_addr[0]) {
        case DS18S20:
            sensor->max_value   = 50.0F;        //TODO
            sensor->min_value   = 0.0F;        //TODO
            sensor->resolution  = 2.0F;        //TODO
            break;
        case DS18B20:
            sensor->max_value   = 257.0F;
            sensor->min_value   = -67.0F;
            sensor->resolution  = 1.0F;
            break;
        case DS1822:
            sensor->max_value   = 257.0F;
            sensor->min_value   = -67.0F;
            sensor->resolution  = 3.5F;
            break;
        case DS2438:
            sensor->max_value   = 125.0F;        //TODO
            sensor->min_value   = -40.0F;        //TODO
            sensor->resolution  = 0.1F;        //TODO
            break;
        default:
            // Unknown type, default to 0.
            sensor->max_value   = 0.0F;
            sensor->min_value   = 0.0F;
            sensor->resolution  = 0.0F;
            break;
    }
}

/**************************************************************************/
/*!
 @brief  Reads the sensor and returns the data as a sensors_event_t
 */
/**************************************************************************/
void PietteTech_DSX_U::getEvent(sensors_event_t *event)
{
    // Clear event definition.
    memset(event, 0, sizeof(sensors_event_t));
    // Populate sensor reading values.
    event->version     = sizeof(sensors_event_t);
    event->sensor_id   = _sensorID;
    event->type        = SENSOR_TYPE_AMBIENT_TEMPERATURE;
    event->timestamp   = millis();
    event->temperature = getTemperature();
}
