/*
 *  PietteTech_Sensor.cpp
 *
 *  Adaption of Adafruit Unified driver to Spark Core
 *
 *  Adapted by Scott Piette (Piette Technologies, LTD)
 *  for the Open Source Beehives Project (http://www.opensourcebeehives.net)
 *
 *  Updated by S. Piette (Piette Technologies)
 * 	Added Serial.print methods for output of the sensor and event details
 *
 *  Intentionally modeled after Adafruit_Sensor.cpp
 *  https://github.com/adafruit/Adafruit_Sensor/blob/master/include/Adafruit_Sensor.h
 *
 */

#include "PietteTech_Sensor.h"
#include <math.h>	// for isnan()

void PietteTech_Sensor::constructor() {}

/**************************************************************************/
/*!
        Function:  printSensorUnits
        Print the Unified Sensor Units
*/
/**************************************************************************/
void PietteTech_Sensor::printSensorUnits(uint32_t sensor_type, bool nl) {
    switch (sensor_type) {
        case SENSOR_TYPE_AMBIENT_TEMPERATURE:
            if (nl)
                Serial.println(" *C");
            else
                Serial.print(" *C");
            break;
        case SENSOR_TYPE_RELATIVE_HUMIDITY:
            if (nl)
                Serial.println(" %");
            else
                Serial.print(" %");
            break;
        case SENSOR_TYPE_PRESSURE:
            if (nl)
                Serial.println(" mPa");
            else
                Serial.print(" mPa");
            break;
    }
}

/**************************************************************************/
/*!
        Function:  printSensorType
        Print unified sensor data description
*/
/**************************************************************************/
void PietteTech_Sensor::printSensorType(uint32_t sensor_type, bool nl) {
    switch (sensor_type) {
        case SENSOR_TYPE_AMBIENT_TEMPERATURE:
            if (nl)
                Serial.println("Temperature");
            else
                Serial.print("Temperature");
            break;
        case SENSOR_TYPE_RELATIVE_HUMIDITY:
            if (nl)
                Serial.println("Humidity");
            else
                Serial.print("Humidity");
            break;
        case SENSOR_TYPE_PRESSURE:
            if (nl)
                Serial.println("Pressure");
            else
                Serial.print("Pressure");
            break;
    }
}

/**************************************************************************/
/*
        Function:  printSensorDetail
        Print unified sensor detail information
 */
/**************************************************************************/
void PietteTech_Sensor::printSensorDetail(sensor_t *sensor) {
    // Print temperature sensor details.
    Serial.println("------------------------------------");
    Serial.print  ("Sensor:       "); Serial.println(sensor->name);
    Serial.print  ("Type:         "); printSensorType(sensor->type, true);
    Serial.print  ("Driver Ver:   "); Serial.println(sensor->version);
    Serial.print  ("Unique ID:    "); Serial.println(sensor->sensor_id);
    Serial.print  ("Max Value:    "); Serial.print(sensor->max_value);
    printSensorUnits(sensor->type, true);
    Serial.print  ("Min Value:    "); Serial.print(sensor->min_value);
    printSensorUnits(sensor->type, true);
    Serial.print  ("Resolution:   "); Serial.print(sensor->resolution);
    printSensorUnits(sensor->type, true);
    Serial.println("------------------------------------");
}

/**************************************************************************/
/*!
        Function:  printSensorEvent
        Print unified sensor event information
*/
/**************************************************************************/
void PietteTech_Sensor::printSensorEvent(sensors_event_t* event, char* name) {
    Serial.println("------------------------------------");
    if (name)
        Serial.print  ("Sensor:       "); Serial.println(name);
    Serial.print  ("Type:         "); printSensorType(event->type, true);
    Serial.print  ("Unique ID:    "); Serial.println(event->sensor_id);
    Serial.print  ("Value:        ");
    if (isnan(event->data[0])) {
        Serial.println("Read Error!");
    }
    else {
        Serial.print(event->data[0]); printSensorUnits(event->type, true);
    }
    Serial.println("------------------------------------");
}
