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
 * Pin for STM32Fxxx is the same as set with @ref OneWireStruct library.
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
 - TM OneWireStruct
 - TM GPIO
 - defines.h
\endverbatim
 */

#include <modules/onewire/onewire.h>

/* OneWireStruct version check */
// #if ONEWIRE_H < 100
// #error "Please update TM OneWireStruct LIB, minimum required version is 2.0.0. Download available on stm32f4-discovery.net website"
// #endif

/**
 * @defgroup DS18B20_Macros
 * @brief    Library defines
 * @{
 */

/* More details: https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf */

/* Every OneWireStruct chip has different ROM code, but all the same chips has same family code */
/* in case of DS18B20 this is 0x28 and this is first byte of ROM address */
#define DS18B20_FAMILY_CODE				0x28

/* DS18B20 Commands */
#define DS18B20_CMD_ALARMSEARCH			0xEC
#define DS18B20_CMD_CONVERTTEMP         0x44    /* Convert temperature */

/* DS18B20 Resolution defines */
#define DS18B20_DECIMAL_STEPS_12BIT		0.0625
#define DS18B20_DECIMAL_STEPS_11BIT		0.125
#define DS18B20_DECIMAL_STEPS_10BIT		0.25
#define DS18B20_DECIMAL_STEPS_9BIT		0.5

/* Bits locations for resolution */
#define DS18B20_RESOLUTION_R1           6
#define DS18B20_RESOLUTION_R0           5

/* CRC enabled */
#ifdef DS18B20_USE_CRC  
#define DS18B20_DATA_LEN                9
#else
#define DS18B20_DATA_LEN                2
#endif

/* DS18B20 Common Numbers */
#define DS18B20_DATA_CRC_BYTE  8
#define DS18B20_DATA_LSB       0
#define DS18B20_DATA_MSB       1

#define DS18B20_MAX_TEMP_DEG_C 125
#define DS18B20_MIN_TEMP_DEG_C -55

#define DS18B20_CONFIG_REGISTER_BYTE          4
#define DS18B20_CONFIG_REGISTER_R0_R1_BITMASK 0x60
#define DS18B20_CONFIG_REGISTER_RESERVED_BITS 5

#define DS18B20_TEMP_SIGN_BITMASK             0x8000

/* Return Codes*/
#define DS18B20_USAGE_ERROR           -1
#define DEVICE_NOT_DS18B20             0
#define DS18B20_CONVERSION_SUCCESS     1
#define DS18B20_CONVERSION_IN_PROGRESS 2
#define DS18B20_CRC_INVALID            3

/**
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
	DS18B20_Resolution_9bits  = 9,   /*!< DS18B20 9 bits resolution */
	DS18B20_Resolution_10bits = 10, /*!< DS18B20 10 bits resolution */
	DS18B20_Resolution_11bits = 11, /*!< DS18B20 11 bits resolution */
	DS18B20_Resolution_12bits = 12  /*!< DS18B20 12 bits resolution */
} DS18B20_Resolution_t;

/**
 * @}
 */

/**
 * @defgroup DS18B20_Functions
 * @brief    Library Functions
 * @{
 */

/**
 * @brief  Starts temperature conversion for specific DS18B20 on specific OneWireStruct channel
 * @param  *OneWireStruct: Pointer to @ref OneWire_t working structure (OneWireStruct channel)
 * @param  *ROM: Pointer to first byte of ROM address for desired DS12B80 device.
 *         Entire ROM address is 8-bytes long
 * @retval:
 *          - DS18B20_USAGE_ERROR           
 *          - DEVICE_NOT_DS18B20            
 *          - 1 : Started DS18B20 at ROM address 
 */
char DS18B20_Start(OneWire_t* OneWireStruct, uint8_t* ROM);

/**
 * @brief  Starts temperature conversion for all DS18B20 devices on specific OneWireStruct channel
 * @note   This mode will skip ROM addressing
 * @param  *OneWireStruct: Pointer to @ref OneWire_t working structure (OneWireStruct channel)
 * @retval:
 *          - DS18B20_USAGE_ERROR           
 *          - DEVICE_NOT_DS18B20            
 *          - 1 : Started all DS18B20s on OneWire Bus   
 */
char DS18B20_StartAll(OneWire_t* OneWireStruct);

/**
 * @brief  Reads temperature from DS18B20
 * @param  *OneWireStruct: Pointer to @ref OneWire_t working structure (OneWireStruct channel)
 * @param  *ROM: Pointer to first byte of ROM address for desired DS12B80 device.
 *         Entire ROM address is 8-bytes long
 * @param  *destination: Pointer to float variable to store temperature
 * @retval:
 *          - DS18B20_USAGE_ERROR           
 *          - DEVICE_NOT_DS18B20            
 *          - DS18B20_CONVERSION_SUCCESS    
 *          - DS18B20_CONVERSION_IN_PROGRESS
 *          - DS18B20_CRC_INVALID           
 */
char DS18B20_Read(OneWire_t* OneWireStruct, uint8_t* ROM, float* destination);

/**
 * @brief  Gets resolution for temperature conversion from DS18B20 device
 * @param  *OneWireStruct: Pointer to @ref OneWire_t working structure (OneWireStruct channel)
 * @param  *ROM: Pointer to first byte of ROM address for desired DS12B80 device.
 *         Entire ROM address is 8-bytes long
 * @retval:
 *          - DS18B20_USAGE_ERROR           
 *          - DEVICE_NOT_DS18B20            
 *          - 9 - 12: Resolution of DS18B20
 */
char DS18B20_GetResolution(OneWire_t* OneWireStruct, uint8_t* ROM);

/**
 * @brief  Sets resolution for specific DS18B20 device
 * @param  *OneWireStruct: Pointer to @ref OneWire_t working structure (OneWireStruct channel)
 * @param  *ROM: Pointer to first byte of ROM address for desired DS12B80 device.
 *         Entire ROM address is 8-bytes long
 * @param  resolution: Resolution for DS18B20 device. This parameter can be a value of @ref DS18B20_Resolution_t enumeration.
 * @retval:
 *            - DS18B20_USAGE_ERROR           
 *            - DEVICE_NOT_DS18B20  
 *            - 1: Resolution set OK
 */
char DS18B20_SetResolution(OneWire_t* OneWireStruct, uint8_t* ROM, DS18B20_Resolution_t resolution);

/**
 * @brief  Checks if device with specific ROM number is DS18B20
 * @param  *ROM: Pointer to first byte of ROM address for desired DS12B80 device.
 *         Entire ROM address is 8-bytes long
 * @retval Device status:
 *            - DS18B20_USAGE_ERROR           
 *            - DEVICE_NOT_DS18B20      
 *            - 1: Device is a DS18B20
 */
char DS18B20_Is(uint8_t* ROM);

/**
 * @brief  Sets high alarm temperature to specific DS18B20 sensor
 * @param  *OneWireStruct: Pointer to @ref OneWire_t working structure (OneWireStruct channel)
 * @param  *ROM: Pointer to first byte of ROM address for desired DS12B80 device.
 *         Entire ROM address is 8-bytes long
 * @param  temp: integer value for temperature between -55 to 125 degrees
 * @retval:
 *            - DS18B20_USAGE_ERROR           
 *            - DEVICE_NOT_DS18B20
 *            - 1: High alarm set OK
 */
char DS18B20_SetAlarmHighTemperature(OneWire_t* OneWireStruct, uint8_t* ROM, int8_t temp);

/**
 * @brief  Sets low alarm temperature to specific DS18B20 sensor
 * @param  *OneWireStruct: Pointer to @ref OneWire_t working structure (OneWireStruct channel)
 * @param  *ROM: Pointer to first byte of ROM address for desired DS12B80 device.
 *         Entire ROM address is 8-bytes long
 * @param  temp: integer value for temperature between -55 to 125 degrees
 * @retval:
 *            - DS18B20_USAGE_ERROR           
 *            - DEVICE_NOT_DS18B20
 *            - 1: Low alarm set OK
 */
char DS18B20_SetAlarmLowTemperature(OneWire_t* OneWireStruct, uint8_t* ROM, int8_t temp);

/**
 * @brief  Disables alarm temperature for specific DS18B20 sensor
 * @param  *OneWireStruct: Pointer to @ref OneWire_t working structure (OneWireStruct channel)
 * @param  *ROM: Pointer to first byte of ROM address for desired DS12B80 device.
 *         Entire ROM address is 8-bytes long
 * @retval:
 *            - DS18B20_USAGE_ERROR           
 *            - DEVICE_NOT_DS18B20
 *            - 1: Alarm disabled OK
 */
char DS18B20_DisableAlarmTemperature(OneWire_t* OneWireStruct, uint8_t* ROM);

/**
 * @brief  Searches for devices with alarm flag set
 * @param  *OneWireStruct: Pointer to @ref OneWire_t working structure (OneWireStruct channel)
 * @retval Alarm search status
 *            - 0: No device found with alarm flag set
 *            - > 0: Device is found with alarm flag
 * @note   To get all devices on one OneWireStruct channel with alarm flag set, you can do this:
\verbatim
while (DS18B20_AlarmSearch(&OneWireStruct)) {
	//Read device ID here 
	//Print to user device by device
}
\endverbatim 
 * @retval:
 *            - DS18B20_USAGE_ERROR           
 *            - 1: Alarm Found
 */
char DS18B20_AlarmSearch(OneWire_t* OneWireStruct);

/**
 * @brief  Checks if all DS18B20 sensors are done with temperature conversion
 * @param  *OneWireStruct: Pointer to @ref OneWire_t working structure (OneWireStruct channel)
 * @retval:
 *            - 0: Not all devices are done with conversion
 *            - DS18B20_CONVERSION_SUCCESS
 */
char DS18B20_AllDone(OneWire_t* OneWireStruct);

/**
 * @}
 */
 
/**
 * @}
 */
 
/**
 * @}
 */
 
#endif

