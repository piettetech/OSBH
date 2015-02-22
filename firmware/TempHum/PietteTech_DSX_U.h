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

#ifndef __PIETTETECH_DSX_H__
#define __PIETTETECH_DSX_H__

#include "PietteTech_Sensor.h"
#include "OneWire.h"

/*=========================================================================
 SENSOR TYPES
 -----------------------------------------------------------------------*/
enum
{
    DS18S20             = 0x10,
    DS18B20             = 0x28,
    DS1822              = 0x22,
    DS2438              = 0x26
};
/*=========================================================================*/



/*=========================================================================
 SCRATCHPAD REGISTERS
 -----------------------------------------------------------------------*/
enum
{
    DSX_REGISTER_TLSB            = 0,     // Temperature LSB
    DSX_REGISTER_TMSB            = 1,     // Temperature MSB
    DSX_REGISTER_TH_UB1          = 2,     // Th register or User Byte 1
    DSX_REGISTER_TL_UB2          = 3,     // Th register or User Byte 1
    DSX_REGISTER_CFG             = 4,     // Configuration Register
    DSX_REGISTER_RESERV1         = 5,     // Reserved (0xFF)
    DSX_REGISTER_RESERV2         = 6,     // Reserved
    DSX_REGISTER_RESERV3         = 7,     // Reserved (0x10)
    DSX_REGISTER_CRC             = 8      // CRC
};
/*=========================================================================*/

/*=========================================================================
 ONE WIRE COMMANDS
 -----------------------------------------------------------------------*/
enum
{
    DSX_CMD_STARTCONVERSION         = 0x44,
    DSX_CMD_READSCRATCHPAD          = 0xBE
};
/*=========================================================================*/

class PietteTech_DSX_U : public PietteTech_Sensor
{
public:
    PietteTech_DSX_U();
    
    void  begin(uint8_t *addr, OneWire *one, int32_t sensorId = -1, char *sensorName = NULL);
    float getTemperature();
    void  getEvent(sensors_event_t*);
    void  getSensor(sensor_t*);
    
private:
    void  setMinDelay(sensor_t* sensor);
    void  setName(sensor_t* sensor);
    char    _name[12];                        /**< sensor name */
    uint8_t  _addr[8];
    OneWire  *_one;
    int32_t  _sensorID;
};

#endif
