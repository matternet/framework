/**	
 * |----------------------------------------------------------------------
 * | Copyright (c) 2016 Tilen MAJERLE
 * |  
 * | Permission is hereby granted, free of charge, to any person
 * | obtaining a copy of this software and associated documentation
 * | files (the "Software"), to deal in the Software without restriction,
 * | including without limitation the rights to use, copy, modify, merge,
 * | publish, distribute, sublicense, and/or sell copies of the Software, 
 * | and to permit persons to whom the Software is furnished to do so, 
 * | subject to the following conditions:
 * | 
 * | The above copyright notice and this permission notice shall be
 * | included in all copies or substantial portions of the Software.
 * | 
 * | THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * | EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * | OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * | AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * | HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * | WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * | FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * | OTHER DEALINGS IN THE SOFTWARE.
 * |----------------------------------------------------------------------
 */
#include "ds18b20.h"

/**
 * @brief  Private function converts config register value to uint8_t resolution
 * @param  *OneWireStruct: Pointer to @ref OneWire_t working structure (OneWire channel)
 * @return None
 */
uint8_t DS18B20_ConfigRegToResolution(uint8_t config_register) {
    return ((config_register & DS18B20_CONFIG_REGISTER_R0_R1_BITMASK) 
        >> DS18B20_CONFIG_REGISTER_RESERVED_BITS) + 9;
}

DS18B20_Status DS18B20_Start(OneWire_t* OneWire, uint8_t *ROM) {
    if (!OneWire || !ROM) return DS18B20_USAGE_ERROR;
	if (DS18B20_Is(ROM)==DEVICE_NOT_DS18B20) return DEVICE_NOT_DS18B20;
	
	/* Reset line */
	OneWire_Reset(OneWire);
	/* Select ROM number */
	OneWire_SelectWithPointer(OneWire, ROM);
	/* Start temperature conversion */
	OneWire_WriteByte(OneWire, DS18B20_CMD_CONVERTTEMP);
	
	return DS18B20_SUCCESS;
}

DS18B20_Status DS18B20_StartAll(OneWire_t* OneWire) {
    if (!OneWire) return DS18B20_USAGE_ERROR;
	/* Reset pulse */
	OneWire_Reset(OneWire);
	/* Skip rom */
	OneWire_WriteByte(OneWire, ONEWIRE_CMD_SKIPROM);
	/* Start conversion on all connected devices */
	OneWire_WriteByte(OneWire, DS18B20_CMD_CONVERTTEMP);
    return DS18B20_SUCCESS;
}

DS18B20_Status DS18B20_Read(OneWire_t* OneWire, uint8_t *ROM, float *destination) {
    if (!OneWire || !ROM || !destination) return DS18B20_USAGE_ERROR;

	uint16_t temperature;
	uint8_t resolution;
	int8_t digit;
    bool minus = false;
	float decimal;
	uint8_t i = 0;
	uint8_t data[DS18B20_READ_DATA_SIZE];
	uint8_t crc;
	
	/* Check if device is DS18B20 */
	if (DS18B20_Is(ROM)==DEVICE_NOT_DS18B20) {
		return DEVICE_NOT_DS18B20;
	}
	
	/* Check if line is released, if it is, then conversion is complete */
    uint8_t is_conversion_done = 0;
    OneWire_ReadBit(OneWire,&is_conversion_done);
	if (!is_conversion_done) {
		/* Conversion is not finished yet */
		return DS18B20_CONVERSION_IN_PROGRESS; 
	}

	/* Reset line */
	OneWire_Reset(OneWire);
	/* Select ROM number */
	OneWire_SelectWithPointer(OneWire, ROM);
	/* Read scratchpad command by onewire protocol */
	OneWire_WriteByte(OneWire, ONEWIRE_CMD_RSCRATCHPAD);
	
	/* Get data */
	for (i = 0; i < DS18B20_READ_DATA_SIZE; i++) {
		/* Read byte by byte */
		OneWire_ReadByte(OneWire, &data[i]);
	}
	
	/* Calculate CRC */
	// OneWire_CRC8(data, 8, &crc);
	crc = OneWire_LookupCRC8(data,8);

	/* Check if CRC is ok */
	if (crc != data[DS18B20_READ_CRC_BYTE]) {
		/* CRC invalid */
		return DS18B20_FAILURE;
	}
	
	/* First two bytes of scratchpad are temperature values */
	temperature = data[DS18B20_DATA_LSB] | (data[DS18B20_DATA_MSB] << DS18B20_READ_CRC_BYTE);

	/* Reset line */
	OneWire_Reset(OneWire);
	
	/* Check if temperature is negative */
	if (temperature & DS18B20_TEMP_SIGN_BITMASK) {
		/* Two's complement, temperature is negative */
		temperature = ~temperature + 1;
		minus = true;
	}

	/* Get sensor resolution */
    resolution = DS18B20_ConfigRegToResolution(data[DS18B20_CONFIG_REGISTER_BYTE]);
	

    /* Measure temperature by first extracting the digit (non decimal) portion. */
    /* Ignore all but 1 signed bit */
    digit = temperature >> 4;
    /* extract signed bit and 3 digit temp bits from MSB, put in appropriate power */
    digit |= ((temperature >> 8) & 0x7) << 4;
	
    /* Store decimal digits */
    /* Resolution may be either 9, 10, 11, or 12 bits.
    If the DS18B20  is  configured for  12-bit resolution, all bits in the temperature
    register will contain valid  data. For 11-bit resolution, bit 0 is undefined. 
    For  10-bit resolution, bits 1 and 0 are undefined, and for 9-bit resolution  
    bits  2,  1,  and  0  are  undefined. Shift as needed to extract 1 step of 
    the decimal temperature then multiply by decimal steps per resolution */
	switch (resolution) {
		case DS18B20_RESOLUTION_9BITS: {
			decimal = (temperature >> 3) & 0x01;
			decimal *= (float)DS18B20_DECIMAL_STEPS_9BIT;
            break;
		} 
		case DS18B20_RESOLUTION_10BITS: {
			decimal = (temperature >> 2) & 0x03;
			decimal *= (float)DS18B20_DECIMAL_STEPS_10BIT;
            break;
		}
		case DS18B20_RESOLUTION_11BITS: {
			decimal = (temperature >> 1) & 0x07;
			decimal *= (float)DS18B20_DECIMAL_STEPS_11BIT;
            break;
		} 
		case DS18B20_RESOLUTION_12BITS: {
			decimal = temperature & 0x0F;
			decimal *= (float)DS18B20_DECIMAL_STEPS_12BIT;
            break;
		}
		default: {
			decimal = 0xFF;
			digit = 0;
            break;
		}
	}
	
    /* combine the digit and decimal parts for full float */
	decimal = digit + decimal;
	/* Check for negative part */
    if (minus) {
		decimal *= -1;
	}
	
	/* Set to pointer */
	*destination = decimal;
	
	/* Return 1, temperature valid */
	return DS18B20_SUCCESS;
}

DS18B20_Status DS18B20_GetResolution(OneWire_t* OneWire, uint8_t *ROM, DS18B20_Resolution_t* resolution) {
    if (!OneWire || !ROM || !resolution) return DS18B20_USAGE_ERROR;
	if (DS18B20_Is(ROM)==DEVICE_NOT_DS18B20) return DEVICE_NOT_DS18B20;
    
    uint8_t conf_register = 0, temp = 0;
	
	/* Reset line */
	OneWire_Reset(OneWire);
	/* Select ROM number */
	OneWire_SelectWithPointer(OneWire, ROM);
	/* Read scratchpad command by onewire protocol */
	OneWire_WriteByte(OneWire, ONEWIRE_CMD_RSCRATCHPAD);
	
	/* Ignore first 4 bytes */
	OneWire_ReadByte(OneWire, &temp);
	OneWire_ReadByte(OneWire, &temp);
	OneWire_ReadByte(OneWire, &temp);
	OneWire_ReadByte(OneWire, &temp);
	
	/* 5th byte of scratchpad is configuration register */
	OneWire_ReadByte(OneWire, &conf_register);
	
	/* Return 9 - 12 value according to number of bits */
	*resolution = DS18B20_ConfigRegToResolution(conf_register);
    return DS18B20_SUCCESS;
}

DS18B20_Status DS18B20_SetResolution(OneWire_t* OneWire, uint8_t *ROM, DS18B20_Resolution_t resolution) {
    if (!OneWire || !ROM ) return DS18B20_USAGE_ERROR;

	uint8_t trigger_register_hi, trigger_register_lo, conf_register, temp;
    if (DS18B20_Is(ROM)==DEVICE_NOT_DS18B20) {
		return DEVICE_NOT_DS18B20;
	}
	
	/* Reset line */
	OneWire_Reset(OneWire);
	/* Select ROM number */
	OneWire_SelectWithPointer(OneWire, ROM);
	/* Read scratchpad command by onewire protocol */
	OneWire_WriteByte(OneWire, ONEWIRE_CMD_RSCRATCHPAD);
	
	/* Ignore first 2 bytes */
	OneWire_ReadByte(OneWire, &temp);
	OneWire_ReadByte(OneWire, &temp);
	
	OneWire_ReadByte(OneWire, &trigger_register_hi);
	OneWire_ReadByte(OneWire, &trigger_register_lo);
	OneWire_ReadByte(OneWire, &conf_register);
	
    /*  
    ------------------------------------------
    Configuration Register Resolution Values
    ------------------------------------------
    R0  R1  RESOLUTION_BITS
    0   0   9
    0   1   10
    1   0   11
    1   1   12
    ------------------------------------------
    */	
    switch (resolution) {
        case DS18B20_RESOLUTION_9BITS: {
            conf_register &= ~(1 << DS18B20_RESOLUTION_R1);
            conf_register &= ~(1 << DS18B20_RESOLUTION_R0);
            break;
        } 
        case DS18B20_RESOLUTION_10BITS: {
            conf_register &= ~(1 << DS18B20_RESOLUTION_R1);
            conf_register |= 1 << DS18B20_RESOLUTION_R0;
            break;
        } 
        case DS18B20_RESOLUTION_11BITS: {
            conf_register |= 1 << DS18B20_RESOLUTION_R1;
            conf_register &= ~(1 << DS18B20_RESOLUTION_R0);
            break;
        } 
        case DS18B20_RESOLUTION_12BITS: {
            conf_register |= 1 << DS18B20_RESOLUTION_R1;
            conf_register |= 1 << DS18B20_RESOLUTION_R0;
            break;
        }
        default: {
            return DS18B20_FAILURE;
            break;
        }
    }

	/* Reset line */
	OneWire_Reset(OneWire);
	/* Select ROM number */
	OneWire_SelectWithPointer(OneWire, ROM);
	/* Write scratchpad command by onewire protocol, only trigger_register_hi, trigger_register_lo and conf_register register can be written */
	OneWire_WriteByte(OneWire, ONEWIRE_CMD_WSCRATCHPAD);
	
	/* Write bytes */
	OneWire_WriteByte(OneWire, trigger_register_hi);
	OneWire_WriteByte(OneWire, trigger_register_lo);
	OneWire_WriteByte(OneWire, conf_register);
	
	/* Reset line */
	OneWire_Reset(OneWire);
	/* Select ROM number */
	OneWire_SelectWithPointer(OneWire, ROM);
	/* Copy scratchpad to EEPROM of DS18B20 */
	OneWire_WriteByte(OneWire, ONEWIRE_CMD_CPYSCRATCHPAD);
	
    return DS18B20_SUCCESS;
}

DS18B20_Status DS18B20_Is(uint8_t* ROM) {
    if (!ROM) return DS18B20_USAGE_ERROR;
	/* Checks if first byte is equal to DS18B20's family code */
	if (*ROM != DS18B20_FAMILY_CODE) {
		return DEVICE_NOT_DS18B20;
	}
	return DS18B20_SUCCESS;
}

DS18B20_Status DS18B20_SetAlarmLowTemperature(OneWire_t* OneWire, uint8_t *ROM, int8_t temp) {
    if (!OneWire || !ROM ) return DS18B20_USAGE_ERROR;

	uint8_t trigger_register_lo, trigger_register_hi, conf_register, ignore;
	if (DS18B20_Is(ROM)==DEVICE_NOT_DS18B20) {
		return DEVICE_NOT_DS18B20;
	}
    if (temp > DS18B20_MAX_TEMP_DEG_C) {
        temp = DS18B20_MAX_TEMP_DEG_C;
    } 
    if (temp < DS18B20_MIN_TEMP_DEG_C) {
        temp = DS18B20_MIN_TEMP_DEG_C;
    }
	/* Reset line */
	OneWire_Reset(OneWire);
	/* Select ROM number */
	OneWire_SelectWithPointer(OneWire, ROM);
	/* Read scratchpad command by onewire protocol */
	OneWire_WriteByte(OneWire, ONEWIRE_CMD_RSCRATCHPAD);
	
	/* Ignore first 2 bytes */
	OneWire_ReadByte(OneWire, &ignore);
	OneWire_ReadByte(OneWire, &ignore);
	
	OneWire_ReadByte(OneWire, &trigger_register_hi);
    OneWire_ReadByte(OneWire, &trigger_register_lo);
	OneWire_ReadByte(OneWire, &conf_register);
	
	trigger_register_lo = (uint8_t)temp; 

	/* Reset line */
	OneWire_Reset(OneWire);
	/* Select ROM number */
	OneWire_SelectWithPointer(OneWire, ROM);
	/* Write scratchpad command by onewire protocol, only trigger_register_hi, trigger_register_lo and conf_register register can be written */
	OneWire_WriteByte(OneWire, ONEWIRE_CMD_WSCRATCHPAD);
	
	/* Write bytes */
	OneWire_WriteByte(OneWire, trigger_register_hi);
	OneWire_WriteByte(OneWire, trigger_register_lo);
	OneWire_WriteByte(OneWire, conf_register);
	
	/* Reset line */
	OneWire_Reset(OneWire);
	/* Select ROM number */
	OneWire_SelectWithPointer(OneWire, ROM);
	/* Copy scratchpad to EEPROM of DS18B20 */
	OneWire_WriteByte(OneWire, ONEWIRE_CMD_CPYSCRATCHPAD);
	
    return DS18B20_SUCCESS;
}

DS18B20_Status DS18B20_SetAlarmHighTemperature(OneWire_t* OneWire, uint8_t *ROM, int8_t temp) {
    if (!OneWire || !ROM) return DS18B20_USAGE_ERROR;
    if (DS18B20_Is(ROM)==DEVICE_NOT_DS18B20) return DEVICE_NOT_DS18B20;

    uint8_t trigger_register_lo, trigger_register_hi, conf_register, ignore;
	
	if (temp > DS18B20_MAX_TEMP_DEG_C) {
		temp = DS18B20_MAX_TEMP_DEG_C;
	} 
	if (temp < DS18B20_MIN_TEMP_DEG_C) {
		temp = DS18B20_MIN_TEMP_DEG_C;
	}
	/* Reset line */
	OneWire_Reset(OneWire);
	/* Select ROM number */
	OneWire_SelectWithPointer(OneWire, ROM);
	/* Read scratchpad command by onewire protocol */
	OneWire_WriteByte(OneWire, ONEWIRE_CMD_RSCRATCHPAD);
	
	/* Ignore first 2 bytes */
	OneWire_ReadByte(OneWire, &ignore);
	OneWire_ReadByte(OneWire, &ignore);
	
    OneWire_ReadByte(OneWire, &trigger_register_hi);
    OneWire_ReadByte(OneWire, &trigger_register_lo);
    OneWire_ReadByte(OneWire, &conf_register);
	
	trigger_register_hi = (uint8_t)temp; 

	/* Reset line */
	OneWire_Reset(OneWire);
	/* Select ROM number */
	OneWire_SelectWithPointer(OneWire, ROM);
	/* Write scratchpad command by onewire protocol, only trigger_register_hi, trigger_register_lo and conf_register register can be written */
	OneWire_WriteByte(OneWire, ONEWIRE_CMD_WSCRATCHPAD);
	
	/* Write bytes */
	OneWire_WriteByte(OneWire, trigger_register_hi);
	OneWire_WriteByte(OneWire, trigger_register_lo);
	OneWire_WriteByte(OneWire, conf_register);
	
	/* Reset line */
	OneWire_Reset(OneWire);
	/* Select ROM number */
	OneWire_SelectWithPointer(OneWire, ROM);
	/* Copy scratchpad to EEPROM of DS18B20 */
	OneWire_WriteByte(OneWire, ONEWIRE_CMD_CPYSCRATCHPAD);
	
	return DS18B20_SUCCESS;
}

DS18B20_Status DS18B20_DisableAlarmTemperature(OneWire_t* OneWire, uint8_t *ROM) {
    if (!OneWire || !ROM) return DS18B20_USAGE_ERROR;
    if (DS18B20_Is(ROM)==DEVICE_NOT_DS18B20) return DEVICE_NOT_DS18B20;
	
    uint8_t trigger_register_lo, trigger_register_hi, conf_register, ignore;
	
	/* Reset line */
	OneWire_Reset(OneWire);
	/* Select ROM number */
	OneWire_SelectWithPointer(OneWire, ROM);
	/* Read scratchpad command by onewire protocol */
	OneWire_WriteByte(OneWire, ONEWIRE_CMD_RSCRATCHPAD);
	
	/* Ignore first 2 bytes */
	OneWire_ReadByte(OneWire, &ignore);
	OneWire_ReadByte(OneWire, &ignore);
	
	OneWire_ReadByte(OneWire, &trigger_register_hi);
	OneWire_ReadByte(OneWire, &trigger_register_lo);
	OneWire_ReadByte(OneWire, &conf_register);
	
	trigger_register_hi = DS18B20_MAX_TEMP_DEG_C;
	trigger_register_lo = (uint8_t) DS18B20_MIN_TEMP_DEG_C;

	/* Reset line */
	OneWire_Reset(OneWire);
	/* Select ROM number */
	OneWire_SelectWithPointer(OneWire, ROM);
	/* Write scratchpad command by onewire protocol, only trigger_register_hi, trigger_register_lo and conf_register register can be written */
	OneWire_WriteByte(OneWire, ONEWIRE_CMD_WSCRATCHPAD);
	
	/* Write bytes */
	OneWire_WriteByte(OneWire, trigger_register_hi);
	OneWire_WriteByte(OneWire, trigger_register_lo);
	OneWire_WriteByte(OneWire, conf_register);
	
	/* Reset line */
	OneWire_Reset(OneWire);
	/* Select ROM number */
	OneWire_SelectWithPointer(OneWire, ROM);
	/* Copy scratchpad to EEPROM of DS18B20 */
	OneWire_WriteByte(OneWire, ONEWIRE_CMD_CPYSCRATCHPAD);
	
    return DS18B20_SUCCESS;
}

DS18B20_Status DS18B20_AlarmSearch(OneWire_t* OneWire) {
    if (!OneWire) return DS18B20_USAGE_ERROR;
	/* Start alarm search */
	return OneWire_Search(OneWire, DS18B20_CMD_ALARMSEARCH);
}

DS18B20_Status DS18B20_AllDone(OneWire_t* OneWire) {
    if (!OneWire) return DS18B20_USAGE_ERROR;
	/* If read bit is low, then device is not finished yet with calculation temperature */
    uint8_t is_conversion_done = 0;
    if (OneWire_ReadBit(OneWire, &is_conversion_done)){
        return DS18B20_SUCCESS;
    }
    return DS18B20_FAILURE;
}


