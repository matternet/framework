/**
 * @author  Tilen MAJERLE
 * @email   tilen@majerle.eu
 * @website http://stm32f4-discovery.net
 * @link    http://stm32f4-discovery.net/2015/07/hal-library-06-ds18b20-for-stm32fxxx/
 * @version v1.0
 * @ide     Keil uVision
 * @license MIT
 * @brief   Library for interfacing DS18B20 temperature sensor from Dallas semiconductors.
 *  
\verbatim
   ----------------------------------------------------------------------
    Copyright (c) 2016 Tilen MAJERLE

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software, 
    and to permit persons to whom the Software is furnished to do so, 
    subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
    AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.
   ----------------------------------------------------------------------
\endverbatim
 */
#ifndef DS18B20_H
#define DS18B20_H

/**
 * @addtogroup STM32Fxxx_HAL_Libraries
 * @{
 */

/**
 * @defgroup DS12820
 * @brief    Library for interfacing DS18B20 temperature sensor from Dallas semiconductors - http://stm32f4-discovery.net/2015/07/hal-library-06-ds18b20-for-stm32fxxx/
 * @{
 *
 * With this you can read temperature, set and get temperature resolution from 9 to 12 bits and check if device is DS18B20.
 * 
 * Pin for STM32Fxxx is the same as set with @ref ONEWIRE library.
 *
 * \par Changelog
 *
\verbatim
 Version 1.0
  - First release
\endverbatim
 *
 * \par Dependencies
 *
\verbatim
 - STM32Fxxx HAL
 - TM ONEWIRE
 - TM GPIO
 - defines.h
\endverbatim
 */

#ifdef UNIT_TEST
#include "onewire.h"
#else
#include <modules/onewire/onewire.h>
#endif // UNIT_TEST

/**
 * @defgroup DS18B20_Macros
 * @brief    Library defines
 * @{
 */

/* More details: https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf */

/* Every onewire chip has different ROM code, but all the same chips has same family code */
/* in case of DS18B20 this is 0x28 and this is first byte of ROM address */
#define DS18B20_FAMILY_CODE             0x28
/* DS18B20 Commands */
#define DS18B20_CMD_ALARMSEARCH         0xEC
#define DS18B20_CMD_CONVERTTEMP         0x44    /* Convert temperature */

/* DS18B20 Resolution defines */
#define DS18B20_DECIMAL_STEPS_12BIT     0.0625
#define DS18B20_DECIMAL_STEPS_11BIT     0.125
#define DS18B20_DECIMAL_STEPS_10BIT     0.25
#define DS18B20_DECIMAL_STEPS_9BIT      0.5

#define DS18B20_READ_TIMEOUT_MS         2000  /* Large timeout in ms for any read. Max read time should be ~1000ms for 12bit resolution */

/* Bits locations for resolution */
#define DS18B20_RESOLUTION_R1           6
#define DS18B20_RESOLUTION_R0           5

/* DS18B20 Common Numbers */
#define DS18B20_READ_DATA_SIZE          9
#define DS18B20_READ_CRC_BYTE           8
#define DS18B20_DATA_LSB                0
#define DS18B20_DATA_MSB                1

#define DS18B20_USE_CRC 1

/* DS18B20 Common Numbers */
#define DS18B20_MAX_CONVERSION_TIME_MS 2000

#define DS18B20_DATA_LSB       0
#define DS18B20_DATA_MSB       1

#define DS18B20_MAX_TEMP_DEG_C 125
#define DS18B20_MIN_TEMP_DEG_C (-55)

#define DS18B20_CONFIG_REGISTER_BYTE          4
#define DS18B20_CONFIG_REGISTER_R0_R1_BITMASK 0x60
#define DS18B20_CONFIG_REGISTER_RESERVED_BITS 5

#define DS18B20_TEMP_SIGN_BITMASK             0x8000/**
 * @}
 */

/**
 * @defgroup DS18B20_Typedefs
 * @brief    Library Typedefs
 * @{
 */

/**
 * @brief  DS18B0 Resolutions available
 */
typedef enum {
    DS18B20_RESOLUTION_9BITS  = 9,  /*!< DS18B20 9 bits resolution */
    DS18B20_RESOLUTION_10BITS = 10, /*!< DS18B20 10 bits resolution */
    DS18B20_RESOLUTION_11BITS = 11, /*!< DS18B20 11 bits resolution */
    DS18B20_RESOLUTION_12BITS = 12  /*!< DS18B20 12 bits resolution */
} DS18B20_Resolution_t;

/**
 * @brief  DS18B0 Return Codes
 */
typedef enum {
    DS18B20_USAGE_ERROR            = -4,  /*!< Function Usage Error, check parameters */
    DS18B20_INVALID_DEVICE         = -3,  /*!< Device does not match DS18B20 Family Code */
    DS18B20_CONVERSION_IN_PROGRESS = -2,  /*!< Sensor still processing information, line is low */
    DS18B20_FAILURE                = -1,  /*!< General operation failure, CRC Invalid */
    DS18B20_SUCCESS                =  0,  /*!< DS18B20 function successful */
} DS18B20_Status;

/**
 * @}
 */

/**
 * @defgroup DS18B20_Functions
 * @brief    Library Functions
 * @{
 */

/**
 * @brief  Starts temperature conversion for specific DS18B20 on specific onewire channel
 * @param  *OneWireStruct: Pointer to @ref OneWire_t working structure (OneWire channel)
 * @param  *ROM: Pointer to first byte of ROM address for desired DS12B80 device.
 *         Entire ROM address is 8-bytes long
 * @return retval: see defintion of DS18B20_Status
 */
DS18B20_Status DS18B20_Start(OneWire_t* OneWireStruct, uint8_t* ROM);

/**
 * @brief  Starts the conversion process AND modifies value of p_temp_degC with measured temperature.
 * @param  *temp_config : address of temp_config struct, with an initialized onewire struct
 * @param  *p_temp_degC: address of float to store measured temperature
 * @return retval: see defintion of temp_sensor_status_t
 */
temp_sensor_status_t DS18B20_Wrapper_Read(temp_config_t* temp_config, float* p_temp_degC); 

/**
 * @brief  Starts temperature conversion for all DS18B20 devices on specific onewire channel
 * @note   This mode will skip ROM addressing
 * @param  *OneWireStruct: Pointer to @ref OneWire_t working structure (OneWire channel)
 * @return retval: see defintion of DS18B20_Status
 */
DS18B20_Status DS18B20_StartAll(OneWire_t* OneWireStruct);

/**
 * @brief  Reads temperature from DS18B20
 * @param  *OneWireStruct: Pointer to @ref OneWire_t working structure (OneWire channel)
 * @param  *ROM: Pointer to first byte of ROM address for desired DS12B80 device.
 *         Entire ROM address is 8-bytes long
 * @param  *destination: Pointer to float variable to store temperature
 * @return retval: see defintion of DS18B20_Status
 */
DS18B20_Status DS18B20_Read(OneWire_t* OneWireStruct, uint8_t* ROM, float* destination);

/**
 * @brief  Gets resolution for temperature conversion from DS18B20 device
 * @param  *OneWireStruct: Pointer to @ref OneWire_t working structure (OneWire channel)
 * @param  *ROM: Pointer to first byte of ROM address for desired DS12B80 device.
 *         Entire ROM address is 8-bytes long
 * @param  resolution: DS18B20_Resolution_t type describing its resolution
 * @return retval: see defintion of DS18B20_Status
 */
DS18B20_Status DS18B20_GetResolution(OneWire_t* OneWireStruct, uint8_t* ROM, DS18B20_Resolution_t* resolution);

/**
 * @brief  Sets resolution for specific DS18B20 device
 * @param  *OneWireStruct: Pointer to @ref OneWire_t working structure (OneWire channel)
 * @param  *ROM: Pointer to first byte of ROM address for desired DS12B80 device.
 *         Entire ROM address is 8-bytes long
 * @param  resolution: Resolution for DS18B20 device. This parameter can be a value of @ref DS18B20_Resolution_t enumeration.
 * @return retval: see defintion of DS18B20_Status
 */
DS18B20_Status DS18B20_SetResolution(OneWire_t* OneWireStruct, uint8_t* ROM, DS18B20_Resolution_t resolution);

/**
 * @brief  Checks if device with specific ROM number is DS18B20
 * @param  *ROM: Pointer to first byte of ROM address for desired DS12B80 device.
 *         Entire ROM address is 8-bytes long
 * @return retval: see defintion of DS18B20_Status
 */
DS18B20_Status DS18B20_Is(uint8_t* ROM);

/**
 * @brief  Sets high alarm temperature to specific DS18B20 sensor
 * @param  *OneWireStruct: Pointer to @ref OneWire_t working structure (OneWire channel)
 * @param  *ROM: Pointer to first byte of ROM address for desired DS12B80 device.
 *         Entire ROM address is 8-bytes long
 * @param  temp: integer value for temperature between -55 to 125 degrees
 * @return retval: see defintion of DS18B20_Status
 */
DS18B20_Status DS18B20_SetAlarmHighTemperature(OneWire_t* OneWireStruct, uint8_t* ROM, int8_t temp);

/**
 * @brief  Sets low alarm temperature to specific DS18B20 sensor
 * @param  *OneWireStruct: Pointer to @ref OneWire_t working structure (OneWire channel)
 * @param  *ROM: Pointer to first byte of ROM address for desired DS12B80 device.
 *         Entire ROM address is 8-bytes long
 * @param  temp: integer value for temperature between -55 to 125 degrees
 * @return retval: see defintion of DS18B20_Status
 */
DS18B20_Status DS18B20_SetAlarmLowTemperature(OneWire_t* OneWireStruct, uint8_t* ROM, int8_t temp);

/**
 * @brief  Disables alarm temperature for specific DS18B20 sensor
 * @param  *OneWireStruct: Pointer to @ref OneWire_t working structure (OneWire channel)
 * @param  *ROM: Pointer to first byte of ROM address for desired DS12B80 device.
 *         Entire ROM address is 8-bytes long
 * @return retval: see defintion of DS18B20_Status
 */
DS18B20_Status DS18B20_DisableAlarmTemperature(OneWire_t* OneWireStruct, uint8_t* ROM);

/**
 * @brief  Searches for devices with alarm flag set
 * @param  *OneWireStruct: Pointer to @ref OneWire_t working structure (OneWire channel)
 * @note   To get all devices on one onewire channel with alarm flag set, you can do this:
\verbatim
while (DS18B20_AlarmSearch(&OneWireStruct)) {
    //Read device ID here 
    //Print to user device by device
}
\endverbatim 
 * @return retval: see defintion of DS18B20_Status
 */
DS18B20_Status DS18B20_AlarmSearch(OneWire_t* OneWireStruct);

/**
 * @brief  Checks if all DS18B20 sensors are done with temperature conversion
 * @param  *OneWireStruct: Pointer to @ref OneWire_t working structure (OneWire channel)
 * @return retval: see defintion of DS18B20_Status
 */
DS18B20_Status DS18B20_AllDone(OneWire_t* OneWireStruct);

/**
 * @}
 */
 
/**
 * @}
 */
 
/**
 * @}
 */
 
#endif /*DS18B20_H*/

