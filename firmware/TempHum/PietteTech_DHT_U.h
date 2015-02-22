/*
 *  PietteTech_DHT_U
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
 * 	Set senseType in setType method to a define in PietteTech_Sensor.h
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
#ifndef DHT_U_H
#define DHT_U_H

#include "PietteTech_Sensor.h"
#include "PietteTech_DHT.h"

#define DHT_SENSOR_VERSION 1

class PietteTech_DHT_U : public PietteTech_Sensor {
public:
    PietteTech_DHT_U();
    
    void begin(PietteTech_DHT* dht, uint8_t dhtType, int32_t sensorId = -1, int32_t senseType = SENSOR_TYPE_AMBIENT_TEMPERATURE, char *sensorName = NULL);
    void getEvent(sensors_event_t* event);
    void getSensor(sensor_t* sensor);

private:
    PietteTech_DHT* _dht;
    char    _name[12];                        /**< sensor name */
    uint8_t _dht_type;
    int32_t _sense_type;
    int32_t _sensorID;
    float getTemperature();
    float getHumidity();
    void  setMinMaxDelay(sensor_t* sensor);
    void  setName(sensor_t* sensor);
    void  getAutoRange(bool enabled);
};

#endif
