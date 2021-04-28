/**
 * |----------------------------------------------------------------------
 * | Copyright (C) Tilen Majerle, 2014
 * |
 * | This program is free software: you can redistribute it and/or modify
 * | it under the terms of the GNU General Public License as published by
 * | the Free Software Foundation, either version 3 of the License, or
 * | any later version.
 * |
 * | This program is distributed in the hope that it will be useful,
 * | but WITHOUT ANY WARRANTY; without even the implied warranty of
 * | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * | GNU General Public License for more details.
 * |
 * | You should have received a copy of the GNU General Public License
 * | along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * |----------------------------------------------------------------------
 */

#ifndef ONEWIRE_H
#define ONEWIRE_H

/* C++ detection */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#ifdef UNIT_TEST
#include "temp_sensor.h"
#else
#include <modules/temp_sensor/temp_sensor.h>
#endif // UNIT_TEST


/* OneWire commands */
#define ONEWIRE_CMD_RSCRATCHPAD         0xBE
#define ONEWIRE_CMD_WSCRATCHPAD         0x4E
#define ONEWIRE_CMD_CPYSCRATCHPAD       0x48
#define ONEWIRE_CMD_RECEEPROM           0xB8
#define ONEWIRE_CMD_RPWRSUPPLY          0xB4
#define ONEWIRE_CMD_SEARCHROM           0xF0
#define ONEWIRE_CMD_READROM             0x33
#define ONEWIRE_CMD_MATCHROM            0x55
#define ONEWIRE_CMD_SKIPROM             0xCC

/* OneWire Constants */
#define ROM_DATA_SIZE_BYTES 8
#define ROM_DATA_SIZE_BITS  (ROM_DATA_SIZE_BYTES * 8)

/* Delay timing for onewire protocol */
/* Do not change these unless you have reviewed the datasheet */
#define ONEWIRE_TX_MIN_RESET_PULSE_TIME_USEC             480  // min time required to pull line low for reset pulse
#define ONEWIRE_WAIT_PRESENCE_PULSE_TIME_USEC            70   // time given to other onewire device before we poll for presence pulse 
#define ONEWIRE_RX_MIN_RESET_PULSE_TIME_USEC             (480 - ONEWIRE_WAIT_PRESENCE_PULSE_TIME_USEC) // fill out remaining time needed for reset
#define ONEWIRE_TX_WRITE_0_BIT_LO_TIME_USEC              65   // 60 < time < 120 us
#define ONEWIRE_TX_WRITE_1_BIT_LO_TIME_USEC              10   // time < 15us, helps to be closer to 15us
#define ONEWIRE_TX_WRITE_1_BIT_HI_TIME_USEC              (60 - ONEWIRE_TX_WRITE_1_BIT_LO_TIME_USEC)    // fill out remainder of write 1 slot time
#define ONEWIRE_TX_RECOVER_TIME_USEC                     5    // any value > 1us
#define ONEWIRE_WAIT_SLAVE_READ_BIT_TIME_USEC            55               
#define ONEWIRE_RX_READ_BIT_LO_TIME_USEC                 1
#define ONEWIRE_RX_READ_BIT_WAIT_BEFORE_SAMPLE_TIME_USEC 5

/* General OneWire return codes */
typedef enum {
    ONEWIRE_FAILURE          =  -1, // Error: Usage / Operation Failure.
    ONEWIRE_SUCCESS          =   0  // Operation Successful
} OneWireStatus;

/**
 * @}
 */
 
/**
 * @defgroup ONEWIRE_Typedefs
 * @brief    Library Typedefs
 * @{
 */

/**
 * @brief  OneWire working struct
 * @note   There are three advanced search variations using the same state information, namely LastDiscrepancy, LastFamilyDiscrepancy, LastDeviceFlag, and ROM_NO.
 * @note   These variations allow specific family types to be targeted or skipped and device present verification 
 * @note   See https://www.maximintegrated.com/en/design/technical-documents/app-notes/1/187.html for more info.
 * @note   All members except ROM_NUM member are fully private and should not be touched by user.
 */
typedef struct OneWire_t {
    uint32_t PalLine;                        /*!< GPIO port to be used for I/O functions */
    uint8_t  LastDiscrepancy;                /*!< Search private */
    uint8_t  LastFamilyDiscrepancy;          /*!< Search private */
    uint8_t  LastDeviceFlag;                 /*!< Search private */
    uint8_t  ROM_NUM[ROM_DATA_SIZE_BYTES];   /*!< 8-bytes address of last search device */
} OneWire_t;

/**
 * @}
 */

/**
 * @defgroup ONEWIRE_Functions
 * @brief    Library Functions
 * @{
 */

/**
 * @brief  Sleep for time_us microseconds
 * @note   Blocking sleep function. Seems to be faster/more accurate than chThdSleepMicroseconds()
 * @note   Min time taken to toggle between OUTPUT -> INPUT ~= 4-5us
 * @note   If you are using OneWire_Delay while toggling output->input should result in
 * @note   total_delay = toggle_time + delay_time
 * @param  time_us: time to put system to sleep
 * @return retval: see defintion of OneWireStatus
 */
void OneWire_Delay(uint32_t time_us);

/* Pin settings */
/**
 * @brief  Set OneWire bus low
 * @param  *OneWireStruct: Pointer to an initialized @ref Onewire_t structure
 * @return retval: see defintion of OneWireStatus
 */
OneWireStatus OneWire_Low(OneWire_t* OneWireStruct);

/**
 * @brief  Set OneWire bus high
 * @param  *OneWireStruct: Pointer to an initialized @ref Onewire_t structure
 * @return retval: see defintion of OneWireStatus
 */
OneWireStatus OneWire_High(OneWire_t* OneWireStruct);

/**
 * @brief  Release line for the onewire bus.
 * @param  *OneWireStruct: Pointer to an initialized @ref Onewire_t structure
 * @return retval: see defintion of OneWireStatus
 */
OneWireStatus OneWire_Input(OneWire_t* OneWireStruct);

/**
 * @brief  Pull line for onewire bus.
 * @param  *OneWireStruct: Pointer to an initialized @ref Onewire_t structure
 * @return retval: see defintion of OneWireStatus
 */
OneWireStatus OneWire_Output(OneWire_t* OneWireStruct);

/**
 * @brief  Wrapper init function to be used with temp_config_t struct;
 * @return retval: see defintion of temp_sensor_status_t
 */
temp_sensor_status_t OneWire_System_Init(temp_config_t* temp_config);

/**
 * @brief  Initializes OneWire struct and set GPIO port.
 * @param  *OneWireStruct: Pointer to a @ref Onewire_t structure
 * @param  PalLine: GPIO Pin to be used for onewire channel
 * @return retval: see defintion of OneWireStatus
 */
OneWireStatus OneWire_Init(OneWire_t* OneWireStruct, uint32_t PalLine);

/**
 * @brief  Resets OneWire bus
 * 
 * @note   Sends reset command for OneWire
 * @param  *OneWireStruct: Pointer to an uninitialized @ref OneWire_t structure
 * @return retval: see defintion of OneWireStatus
 */
OneWireStatus OneWire_Reset(OneWire_t* OneWireStruct);

/**
 * @brief  Reads byte from one wire bus
 * @note   Takes ~585-86 us to complete
 * @param  *OneWireStruct: Pointer to an initialized @ref Onewire_t structure
 * @param  *ReadVal: Pointer to store byte value from read
 * @return retval: see defintion of OneWireStatus
 */
OneWireStatus OneWire_ReadByte(OneWire_t* OneWireStruct, uint8_t* ReadVal);

/**
 * @brief  Writes byte to bus
 * @note   Takes ~515-16 us to complete
 * @param  *OneWireStruct: Pointer to an initialized @ref Onewire_t structure
 * @param  byte: 8-bit value to write over OneWire protocol
 * @return retval: see defintion of OneWireStatus
 */
OneWireStatus OneWire_WriteByte(OneWire_t* OneWireStruct, uint8_t byte);

/**
 * @brief  Writes single bit to onewire bus
 * @note   Takes ReadByte_time / 8 us to complete
 * @param  *OneWireStruct: Pointer to @ref OneWire_t working onewire structure
 * @param  bit: Bit value to send, 1 or 0
 * @return retval: see defintion of OneWireStatus
 */
OneWireStatus OneWire_WriteBit(OneWire_t* OneWireStruct, uint8_t bit);

/**
 * @brief  Reads single bit from one wire bus
 * @note   Takes ReadByte_time / 8 us to complete
 * @param  *OneWireStruct: Pointer to @ref OneWire_t working onewire structure
 * @param  *ReadVal: Pointer to store bit value from read
 * @return retval: see defintion of OneWireStatus
 */
OneWireStatus OneWire_ReadBit(OneWire_t* OneWireStruct, uint8_t* ReadVal);

/**
 * @brief  Searches for OneWire devices on specific Onewire port
 * @note   Not meant for public use. Use @ref OneWire_First and @ref OneWire_Next for this.
 * @param  *OneWireStruct: Pointer to an initialized @ref Onewire_t structure
 * @param  command: Command to write on onewire bus, see #defines for available commands
 * @return retval: see defintion of OneWireStatus
 */
OneWireStatus OneWire_Search(OneWire_t* OneWireStruct, uint8_t command);

/**
 * @brief  Resets search states
 * @param  *OneWireStruct: Pointer to an initialized @ref Onewire_t structure
 * @return retval: see defintion of OneWireStatus
 */
OneWireStatus OneWire_ResetSearch(OneWire_t* OneWireStruct);

/**
 * @brief  Starts search, reset states first
 * @note   When you want to search for ALL devices on one onewire port, you should first use this function.
\code
//...Initialization before
status = OneWire_First(&OneWireStruct);
while (status) {
    //Save ROM number from device
    OneWire_GetFullROM(ROM_Array_Pointer);
    //Check for new device
    status = OneWire_Next(&OneWireStruct);
}
\endcode
 * @param  *OneWireStruct: Pointer to an initialized @ref Onewire_t structure
 * @return retval: see defintion of OneWireStatus
*/
OneWireStatus OneWire_First(OneWire_t* OneWireStruct);

/**
 * @brief  Reads next device
 * @note   Use @ref OneWire_First to start searching
 * @param  *OneWireStruct: Pointer to an initialized @ref Onewire_t structure
 * @return retval: see defintion of OneWireStatus
 */
OneWireStatus OneWire_Next(OneWire_t* OneWireStruct);

/**
 * @brief  Gets ROM number from device from search
 * @param  *OneWireStruct: Pointer to an initialized @ref Onewire_t structure
 * @param  *ROM : Pointer to store ROM value
 * @param  index: Because each device has 8-bytes long ROM address, you have to call this 8 times, to get ROM bytes from 0 to 7
 * @return retval: see defintion of OneWireStatus
 */
OneWireStatus OneWire_GetROM(OneWire_t* OneWireStruct, uint8_t index, uint8_t *ROM);

/**
 * @brief  Gets all 8 bytes ROM value from device from search
 * @param  *OneWireStruct: Pointer to an initialized @ref Onewire_t structure
 * @param  *firstIndex: Pointer to first location for first byte, other bytes are automatically incremented
 * @return retval: see defintion of OneWireStatus
 */
OneWireStatus OneWire_GetFullROM(OneWire_t* OneWireStruct, uint8_t *firstIndex);

/**
 * @brief  Selects specific slave on bus
 * @param  *OneWireStruct: Pointer to an initialized @ref Onewire_t structure
 * @param  *addr: Pointer to first location of 8-bytes long ROM address
 * @return retval: see defintion of OneWireStatus
 */
OneWireStatus OneWire_Select(OneWire_t* OneWireStruct, uint8_t* addr);

/**
 * @brief  Calculates 8-bit CRC for 1-wire devices. 
 * @note   CRC = X^8 + X^5 + X^4 + 1
 * @param  *addr:  Pointer to 8-bit array of data to calculate CRC
 * @param  len:    Number of bytes to check
 * @param  *rslt:  Pointer to store calculated CRC value 
 * @return retval: see defintion of OneWireStatus
 */
OneWireStatus OneWire_CalculateCRC8(uint8_t* addr, uint8_t len, uint8_t* rslt);

/**
 * @brief  Looks up 8-bit CRC for 1-wire devices, in an effort to save MIPS
 * @note   CRC = X^8 + X^5 + X^4 + 1
 * @param  *addr:  Pointer to 8-bit array of data to calculate CRC
 * @param  len:    Number of bytes to check
 * @return retval: 8-bit CRC value for given array, or ONEWIRE_FAILURE on usage error
 */
uint8_t OneWire_LookupCRC8(uint8_t* addr, uint8_t len);

/* Advanced Search Variations */

/**
 * @brief  Preset the search state to first find a particular family type
 * @param  *ROM: Pointer to first byte of ROM address
 * @return retval: see definition of OneWireStatus
 */
OneWireStatus OneWire_TargetSetup(OneWire_t* OneWireStruct, uint8_t family_code);

/**
 * @brief  Verify if a device with a known ROM is connected to onewire
 * @param  *OneWireStruct: Pointer to an initialized @ref Onewire_t structure
 * @return retval: see defintion of OneWireStatus
 */
OneWireStatus OneWire_Verify(OneWire_t* OneWireStruct);

/**
 * @brief  Sets the search state to skip all of the devices that have the family code that was found in the previous search.
 * @param  *OneWireStruct: Pointer to an initialized @ref Onewire_t structure
 * @return retval: see defintion of OneWireStatus
 */
OneWireStatus OneWire_FamilySkipSetup(OneWire_t* OneWireStruct);

/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif /* endif ONEWIRE_H */