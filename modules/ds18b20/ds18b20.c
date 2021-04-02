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

char DS18B20_Start(OneWire_t* OneWireStruct, uint8_t *ROM) {
    if (!OneWireStruct)   return DS18B20_USAGE_ERROR;
    if (!DS18B20_Is(ROM)) return DEVICE_NOT_DS18B20;    

    /* Reset line */
    OneWire_Reset(OneWireStruct);
    /* Select ROM number */
    OneWire_SelectWithPointer(OneWireStruct, ROM);
    /* Start temperature conversion */
    OneWire_WriteByte(OneWireStruct, DS18B20_CMD_CONVERTTEMP);
    
    return 1;
}

char DS18B20_StartAll(OneWire_t* OneWireStruct) {
    if (!OneWireStruct) return DS18B20_USAGE_ERROR;
    /* Reset pulse */
    OneWire_Reset(OneWireStruct);
    /* Skip rom */
    OneWire_WriteByte(OneWireStruct, ONEWIRE_CMD_SKIPROM);
    /* Start conversion on all connected devices */
    OneWire_WriteByte(OneWireStruct, DS18B20_CMD_CONVERTTEMP);
    return 1;
}

char DS18B20_Read(OneWire_t* OneWireStruct, uint8_t *ROM, float *destination) {
    if (!OneWireStruct)   return DS18B20_USAGE_ERROR;
    if (!ROM)             return DS18B20_USAGE_ERROR;
    if (!destination)     return DS18B20_USAGE_ERROR;
    if (!DS18B20_Is(ROM)) return DEVICE_NOT_DS18B20;

    uint16_t temperature;
    unsigned char resolution;
    int8_t digit, minus = 0;
    float decimal;
    unsigned char i = 0;
    unsigned char data[DS18B20_DATA_LEN];
    unsigned char crc;
    
    /* Check if device is DS18B20 */
    
    
    /* Check if line is released, if it is, then conversion is complete */
    if (OneWire_ReadBit(OneWireStruct)==0) {
        /* Conversion is not finished yet */
        return DS18B20_CONVERSION_IN_PROGRESS; 
    }

    /* Reset line */
    OneWire_Reset(OneWireStruct);
    /* Select ROM number */
    OneWire_SelectWithPointer(OneWireStruct, ROM);
    /* Read scratchpad command by OneWireStruct protocol */
    OneWire_WriteByte(OneWireStruct, ONEWIRE_CMD_RSCRATCHPAD);
    
    /* Get data */
    for (i = 0; i < DS18B20_DATA_LEN; i++) {
        /* Read byte by byte */
        data[i] = OneWire_ReadByte(OneWireStruct);
    }
    
    /* Calculate CRC */
    crc = OneWire_CRC8(data, DS18B20_DATA_CRC_BYTE);
    
    /* Check if CRC is ok */
    if (crc != data[DS18B20_DATA_CRC_BYTE]) {
        /* CRC invalid */
        return DS18B20_CRC_INVALID;
    }
    
    /* First two bytes of scratchpad are temperature values */
    temperature = data[DS18B20_DATA_LSB] | (data[DS18B20_DATA_MSB] << 8);

    /* Reset line */
    OneWire_Reset(OneWireStruct);
    
    /* Check if temperature is negative */
    if (temperature & 0x8000) {
        /* Two's complement, temperature is negative */
        temperature = ~temperature + 1;
        minus = 1;
    }

    /* Get sensor resolution */
    /* Extract resolution bits with &, Ignore unused bits in the configuration register 
    which are reserved for internal use by the device. Add 9 to convert R0 and R1 -> 9-12 for 
    bits in resolution */
    resolution = ((data[DS18B20_CONFIG_REGISTER_BYTE] & DS18B20_CONFIG_REGISTER_R0_R1_BITMASK) 
        >> DS18B20_CONFIG_REGISTER_RESERVED_BITS) + 9;
    
    /* Store temperature integer digits and decimal digits */
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
        case 9: {
            decimal = (temperature >> 3) & 0x01;
            decimal *= (float)DS18B20_DECIMAL_STEPS_9BIT;
        } break;
        case 10: {
            decimal = (temperature >> 2) & 0x03;
            decimal *= (float)DS18B20_DECIMAL_STEPS_10BIT;
        } break;
        case 11: {
            decimal = (temperature >> 1) & 0x07;
            decimal *= (float)DS18B20_DECIMAL_STEPS_11BIT;
        } break;
        case 12: {
            decimal = temperature & 0x0F;
            decimal *= (float)DS18B20_DECIMAL_STEPS_12BIT;
        } break;
        default: {
            decimal = 0xFF;
            digit = 0;
        }
    }
    
    /* Check for negative part */
    decimal = digit + decimal;
    if (minus) {
        decimal *= -1;
    }
    
    /* Set to pointer */
    *destination = decimal;
    
    /* Return 1, temperature valid */
    return DS18B20_CONVERSION_SUCCESS;
}

char DS18B20_GetResolution(OneWire_t* OneWireStruct, uint8_t *ROM) {
    if (!OneWireStruct)   return DS18B20_USAGE_ERROR;
    if (!ROM)             return DS18B20_USAGE_ERROR;
    if (!DS18B20_Is(ROM)) return DEVICE_NOT_DS18B20;
    
    char conf_register;
    /* Reset line */
    OneWire_Reset(OneWireStruct);
    /* Select ROM number */
    OneWire_SelectWithPointer(OneWireStruct, ROM);
    /* Read scratchpad command by OneWireStruct protocol */
    OneWire_WriteByte(OneWireStruct, ONEWIRE_CMD_RSCRATCHPAD);
    
    /* Ignore first 4 bytes */
    OneWire_ReadByte(OneWireStruct);
    OneWire_ReadByte(OneWireStruct);
    OneWire_ReadByte(OneWireStruct);
    OneWire_ReadByte(OneWireStruct);
    
    /* 5th byte of scratchpad is configuration register */
    conf_register = OneWire_ReadByte(OneWireStruct);
    
    /* Return 9 - 12 value according to number of bits */
    /* Extract resolution bits with &, Ignore unused bits in the configuration register 
    which are reserved for internal use by the device. Add 9 to convert R0 and R1 -> 9-12 for 
    bits in resolution */
    return = ((data[DS18B20_CONFIG_REGISTER_BYTE] & DS18B20_CONFIG_REGISTER_R0_R1_BITMASK) 
        >> DS18B20_CONFIG_REGISTER_RESERVED_BITS) + 9;
}

char DS18B20_SetResolution(OneWire_t* OneWireStruct, uint8_t *ROM, DS18B20_Resolution_t resolution) {
    if (!OneWireStruct)   return DS18B20_USAGE_ERROR;
    if (!ROM)             return DS18B20_USAGE_ERROR;
    if (!DS18B20_Is(ROM)) return DEVICE_NOT_DS18B20;

    char trigger_register_hi, trigger_register_lo, conf_register;
    /* Reset line */
    OneWire_Reset(OneWireStruct);
    /* Select ROM number */
    OneWire_SelectWithPointer(OneWireStruct, ROM);
    /* Read scratchpad command by OneWireStruct protocol */
    OneWire_WriteByte(OneWireStruct, ONEWIRE_CMD_RSCRATCHPAD);
    
    /* Ignore first 2 bytes */
    OneWire_ReadByte(OneWireStruct);
    OneWire_ReadByte(OneWireStruct);
    
    trigger_register_hi = OneWire_ReadByte(OneWireStruct);
    trigger_register_lo = OneWire_ReadByte(OneWireStruct);
    conf_register = OneWire_ReadByte(OneWireStruct);
    
    if (resolution == DS18B20_Resolution_9bits) {
        conf_register &= ~(1 << DS18B20_RESOLUTION_R1);
        conf_register &= ~(1 << DS18B20_RESOLUTION_R0);
    } else if (resolution == DS18B20_Resolution_10bits) {
        conf_register &= ~(1 << DS18B20_RESOLUTION_R1);
        conf_register |= 1 << DS18B20_RESOLUTION_R0;
    } else if (resolution == DS18B20_Resolution_11bits) {
        conf_register |= 1 << DS18B20_RESOLUTION_R1;
        conf_register &= ~(1 << DS18B20_RESOLUTION_R0);
    } else if (resolution == DS18B20_Resolution_12bits) {
        conf_register |= 1 << DS18B20_RESOLUTION_R1;
        conf_register |= 1 << DS18B20_RESOLUTION_R0;
    }
    
    /* Reset line */
    OneWire_Reset(OneWireStruct);
    /* Select ROM number */
    OneWire_SelectWithPointer(OneWireStruct, ROM);
    /* Write scratchpad command by OneWireStruct protocol, only trigger_register_hi, trigger_register_lo and conf_register register can be written */
    OneWire_WriteByte(OneWireStruct, ONEWIRE_CMD_WSCRATCHPAD);
    
    /* Write bytes */
    OneWire_WriteByte(OneWireStruct, trigger_register_hi);
    OneWire_WriteByte(OneWireStruct, trigger_register_lo);
    OneWire_WriteByte(OneWireStruct, conf_register);
    
    /* Reset line */
    OneWire_Reset(OneWireStruct);
    /* Select ROM number */
    OneWire_SelectWithPointer(OneWireStruct, ROM);
    /* Copy scratchpad to EEPROM of DS18B20 */
    OneWire_WriteByte(OneWireStruct, ONEWIRE_CMD_CPYSCRATCHPAD);
    
    return 1;
}

char DS18B20_Is(uint8_t *ROM) {
    if (!ROM) return DS18B20_USAGE_ERROR;
    /* Checks if first byte is equal to DS18B20's family code */
    if (*ROM == DS18B20_FAMILY_CODE) return 1;
    return 0;
}

char DS18B20_SetAlarmLowTemperature(OneWire_t* OneWireStruct, uint8_t *ROM, int8_t temp) {
    if (!OneWireStruct)   return DS18B20_USAGE_ERROR;
    if (!ROM)             return DS18B20_USAGE_ERROR;
    if (!DS18B20_Is(ROM)) return DEVICE_NOT_DS18B20;

    char trigger_register_lo, trigger_register_hi, conf_register;
    if (temp > DS18B20_MAX_TEMP_DEG_C) {
        temp = DS18B20_MAX_TEMP_DEG_C;
    } 
    if (temp < DS18B20_MIN_TEMP_DEG_C) {
        temp = DS18B20_MIN_TEMP_DEG_C;
    }
    /* Reset line */
    OneWire_Reset(OneWireStruct);
    /* Select ROM number */
    OneWire_SelectWithPointer(OneWireStruct, ROM);
    /* Read scratchpad command by OneWireStruct protocol */
    OneWire_WriteByte(OneWireStruct, ONEWIRE_CMD_RSCRATCHPAD);
    
    /* Ignore first 2 bytes */
    OneWire_ReadByte(OneWireStruct);
    OneWire_ReadByte(OneWireStruct);
    
    trigger_register_hi = OneWire_ReadByte(OneWireStruct);
    trigger_register_lo = OneWire_ReadByte(OneWireStruct);
    conf_register = OneWire_ReadByte(OneWireStruct);
    
    trigger_register_lo = temp; 

    /* Reset line */
    OneWire_Reset(OneWireStruct);
    /* Select ROM number */
    OneWire_SelectWithPointer(OneWireStruct, ROM);
    /* Write scratchpad command by OneWireStruct protocol, only trigger_register_hi, trigger_register_lo and conf_register register can be written */
    OneWire_WriteByte(OneWireStruct, ONEWIRE_CMD_WSCRATCHPAD);
    
    /* Write bytes */
    OneWire_WriteByte(OneWireStruct, trigger_register_hi);
    OneWire_WriteByte(OneWireStruct, trigger_register_lo);
    OneWire_WriteByte(OneWireStruct, conf_register);
    
    /* Reset line */
    OneWire_Reset(OneWireStruct);
    /* Select ROM number */
    OneWire_SelectWithPointer(OneWireStruct, ROM);
    /* Copy scratchpad to EEPROM of DS18B20 */
    OneWire_WriteByte(OneWireStruct, ONEWIRE_CMD_CPYSCRATCHPAD);
    
    return 1;
}

char DS18B20_SetAlarmHighTemperature(OneWire_t* OneWireStruct, uint8_t *ROM, int8_t temp) {
    if (!OneWireStruct)   return DS18B20_USAGE_ERROR;
    if (!ROM)             return DS18B20_USAGE_ERROR;
    if (!DS18B20_Is(ROM)) return DEVICE_NOT_DS18B20;

    char trigger_register_lo, trigger_register_hi, conf_register;

    if (temp > DS18B20_MAX_TEMP_DEG_C) {
        temp = DS18B20_MAX_TEMP_DEG_C;
    } 
    if (temp < DS18B20_MIN_TEMP_DEG_C) {
        temp = DS18B20_MIN_TEMP_DEG_C;
    }
    /* Reset line */
    OneWire_Reset(OneWireStruct);
    /* Select ROM number */
    OneWire_SelectWithPointer(OneWireStruct, ROM);
    /* Read scratchpad command by OneWireStruct protocol */
    OneWire_WriteByte(OneWireStruct, ONEWIRE_CMD_RSCRATCHPAD);
    
    /* Ignore first 2 bytes */
    OneWire_ReadByte(OneWireStruct);
    OneWire_ReadByte(OneWireStruct);
    
    trigger_register_hi = OneWire_ReadByte(OneWireStruct);
    trigger_register_lo = OneWire_ReadByte(OneWireStruct);
    conf_register = OneWire_ReadByte(OneWireStruct);
    
    trigger_register_hi = temp; 

    /* Reset line */
    OneWire_Reset(OneWireStruct);
    /* Select ROM number */
    OneWire_SelectWithPointer(OneWireStruct, ROM);
    /* Write scratchpad command by OneWireStruct protocol, only trigger_register_hi, trigger_register_lo and conf_register register can be written */
    OneWire_WriteByte(OneWireStruct, ONEWIRE_CMD_WSCRATCHPAD);
    
    /* Write bytes */
    OneWire_WriteByte(OneWireStruct, trigger_register_hi);
    OneWire_WriteByte(OneWireStruct, trigger_register_lo);
    OneWire_WriteByte(OneWireStruct, conf_register);
    
    /* Reset line */
    OneWire_Reset(OneWireStruct);
    /* Select ROM number */
    OneWire_SelectWithPointer(OneWireStruct, ROM);
    /* Copy scratchpad to EEPROM of DS18B20 */
    OneWire_WriteByte(OneWireStruct, ONEWIRE_CMD_CPYSCRATCHPAD);
    
    return DS18B20_CONVERSION_SUCCESS;
}

char DS18B20_DisableAlarmTemperature(OneWire_t* OneWireStruct, uint8_t *ROM) {
    if (!OneWireStruct)   return DS18B20_USAGE_ERROR;
    if (!ROM)             return DS18B20_USAGE_ERROR;
    if (!DS18B20_Is(ROM)) return DEVICE_NOT_DS18B20;

    char trigger_register_lo, trigger_register_hi, conf_register;
    /* Reset line */
    OneWire_Reset(OneWireStruct);
    /* Select ROM number */
    OneWire_SelectWithPointer(OneWireStruct, ROM);
    /* Read scratchpad command by OneWireStruct protocol */
    OneWire_WriteByte(OneWireStruct, ONEWIRE_CMD_RSCRATCHPAD);
    
    /* Ignore first 2 bytes */
    OneWire_ReadByte(OneWireStruct);
    OneWire_ReadByte(OneWireStruct);
    
    trigger_register_hi = OneWire_ReadByte(OneWireStruct);
    trigger_register_lo = OneWire_ReadByte(OneWireStruct);
    conf_register = OneWire_ReadByte(OneWireStruct);
    
    trigger_register_hi = DS18B20_MAX_TEMP_DEG_C;
    trigger_register_lo = DS18B20_MIN_TEMP_DEG_C;

    /* Reset line */
    OneWire_Reset(OneWireStruct);
    /* Select ROM number */
    OneWire_SelectWithPointer(OneWireStruct, ROM);
    /* Write scratchpad command by OneWireStruct protocol, only trigger_register_hi, trigger_register_lo and conf_register register can be written */
    OneWire_WriteByte(OneWireStruct, ONEWIRE_CMD_WSCRATCHPAD);
    
    /* Write bytes */
    OneWire_WriteByte(OneWireStruct, trigger_register_hi);
    OneWire_WriteByte(OneWireStruct, trigger_register_lo);
    OneWire_WriteByte(OneWireStruct, conf_register);
    
    /* Reset line */
    OneWire_Reset(OneWireStruct);
    /* Select ROM number */
    OneWire_SelectWithPointer(OneWireStruct, ROM);
    /* Copy scratchpad to EEPROM of DS18B20 */
    OneWire_WriteByte(OneWireStruct, ONEWIRE_CMD_CPYSCRATCHPAD);
    
    return 1;
}

char DS18B20_AlarmSearch(OneWire_t* OneWireStruct) {
    if (!OneWireStruct)   return DS18B20_USAGE_ERROR;
    /* Start alarm search */
    return OneWire_Search(OneWireStruct, DS18B20_CMD_ALARMSEARCH);
}

char DS18B20_AllDone(OneWire_t* OneWireStruct) {
    if (!OneWireStruct)   return DS18B20_USAGE_ERROR;
    /* If read bit is low, then device is not finished yet with calculation temperature */
    return OneWire_ReadBit(OneWireStruct) == DS18B20_CONVERSION_SUCCESS;
}


