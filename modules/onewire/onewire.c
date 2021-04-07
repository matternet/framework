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

#include "onewire.h"

// ONEWIRE ABSTRACTION HELPERS

void OneWire_Delay(uint32_t time_us) {
    usleep(time_us);
}

OneWireStatus OneWire_Low(OneWire_t* OneWireStruct) {
    if (!OneWireStruct) return ONEWIRE_FAILURE;
    palClearLine(OneWireStruct->PalLine);
    return ONEWIRE_SUCCESS;
}

OneWireStatus OneWire_High(OneWire_t* OneWireStruct) {
    if (!OneWireStruct) return ONEWIRE_FAILURE;
    palSetLine(OneWireStruct->PalLine);
    return ONEWIRE_SUCCESS; 
}

OneWireStatus OneWire_Input(OneWire_t* OneWireStruct) {
    if (!OneWireStruct) return ONEWIRE_FAILURE;
    palSetLineMode(OneWireStruct->PalLine, PAL_MODE_INPUT | PAL_STM32_OSPEED_HIGHEST);
    return ONEWIRE_SUCCESS;
}

OneWireStatus OneWire_Output(OneWire_t* OneWireStruct) {
    if (!OneWireStruct) return ONEWIRE_FAILURE;
    palSetLineMode(OneWireStruct->PalLine, PAL_STM32_MODE_OUTPUT | PAL_STM32_OSPEED_HIGHEST);
    return ONEWIRE_SUCCESS;
}

// ONEWIRE IMPLEMENTATION

OneWireStatus OneWire_Init(OneWire_t* OneWireStruct, uint32_t PalLine) {
    if (!OneWireStruct) return ONEWIRE_FAILURE;
    OneWireStruct->PalLine = PalLine;
    return ONEWIRE_SUCCESS;
}

OneWireStatus OneWire_Reset(OneWire_t* OneWireStruct) {
    if (!OneWireStruct) return ONEWIRE_FAILURE;
    uint8_t presence_pulse = 1;
    /* Line low, and wait */
    OneWire_Output(OneWireStruct);
    OneWire_Low(OneWireStruct);
    OneWire_Delay(ONEWIRE_TX_MIN_RESET_PULSE_TIME_USEC);
    
    /* Release line and wait */
    OneWire_Input(OneWireStruct);
    OneWire_Delay(ONEWIRE_WAIT_PRESENCE_PULSE_TIME_USEC);
    
    /* Check bit value */
    presence_pulse = palReadLine(OneWireStruct->PalLine);

    /* Complete reset pulse min time */
    OneWire_Delay(ONEWIRE_RX_MIN_RESET_PULSE_TIME_USEC);
    /* Return value of presence pulse, 0 = OK, 1 = ERROR */
    return (presence_pulse == 0);
}

OneWireStatus OneWire_WriteBit(OneWire_t* OneWireStruct, uint8_t bit) {
    if (!OneWireStruct) return ONEWIRE_FAILURE;
    if (bit) {
        /* Set line low */
        OneWire_Output(OneWireStruct);
        OneWire_Low(OneWireStruct);
        OneWire_Delay(ONEWIRE_TX_WRITE_1_BIT_LO_TIME_USEC);

        /* release line, Hold for remainder of min time for write 1 slot */
        OneWire_Input(OneWireStruct);
        OneWire_Delay(ONEWIRE_TX_WRITE_1_BIT_HI_TIME_USEC);
    } else {
        /* Set line low */
        OneWire_Output(OneWireStruct);
        OneWire_Low(OneWireStruct);
        OneWire_Delay(ONEWIRE_TX_WRITE_0_BIT_LO_TIME_USEC);  

        /* release line, wait for nonzero recovery time */
        OneWire_Input(OneWireStruct);
        OneWire_Delay(ONEWIRE_TX_RECOVER_TIME_USEC);
    }
    return ONEWIRE_SUCCESS;
}

OneWireStatus OneWire_ReadBit(OneWire_t* OneWireStruct, uint8_t* ReadVal) {
    if (!OneWireStruct || !ReadVal) return ONEWIRE_FAILURE;
    uint8_t bit = 0;
    /* Line low */
    OneWire_Output(OneWireStruct);
    OneWire_Low(OneWireStruct);
    OneWire_Delay(ONEWIRE_RX_READ_BIT_LO_TIME_USEC); 

    /* Release line */
    OneWire_Input(OneWireStruct);
    OneWire_Delay(ONEWIRE_RX_READ_BIT_WAIT_BEFORE_SAMPLE_TIME_USEC);

    /* Read line value near end of 15us */
    if (palReadLine(OneWireStruct->PalLine)) {
        /* Bit is HIGH */
        bit = 1;
    }

    /* Wait 50us to complete minimum 60us period */
    OneWire_Delay(50);

    *ReadVal = bit;
    return ONEWIRE_SUCCESS;
}

OneWireStatus OneWire_WriteByte(OneWire_t* OneWireStruct, uint8_t byte) {
    if (!OneWireStruct) return ONEWIRE_FAILURE;
    /* Write 8 bits */
    for (uint8_t i = 0; i < 8; i++) {
        /* LSB bit is first */
        OneWire_WriteBit(OneWireStruct, byte & 0x01);
        byte >>= 1;
    }
    return ONEWIRE_SUCCESS;
}

OneWireStatus OneWire_ReadByte(OneWire_t* OneWireStruct, uint8_t* ReadVal) {
    if (!OneWireStruct || !ReadVal) return ONEWIRE_FAILURE;    
    uint8_t bitval = 0;
    /* Data is transferred LSb first */
    for (uint8_t i = 0; i < 8; i++) {
        OneWire_ReadBit(OneWireStruct, &bitval);  // Could add error-handling here, but unnecessary IMO
        *ReadVal |= (bitval << i);
    }
    return ONEWIRE_SUCCESS;
}

OneWireStatus OneWire_First(OneWire_t* OneWireStruct) {
    if (!OneWireStruct) return ONEWIRE_FAILURE;
    /* Reset search values */
    OneWire_ResetSearch(OneWireStruct);
    /* Start with searching */
    return OneWire_Search(OneWireStruct, ONEWIRE_CMD_SEARCHROM);
}

OneWireStatus OneWire_Next(OneWire_t* OneWireStruct) {
    if (!OneWireStruct) return ONEWIRE_FAILURE;
    /* Leave the search state alone */
    return OneWire_Search(OneWireStruct, ONEWIRE_CMD_SEARCHROM);
}

OneWireStatus OneWire_ResetSearch(OneWire_t* OneWireStruct) {
    if (!OneWireStruct) return ONEWIRE_FAILURE;
    /* Reset the search state */
    OneWireStruct->LastDiscrepancy       = 0;
    OneWireStruct->LastDeviceFlag        = 0;
    OneWireStruct->LastFamilyDiscrepancy = 0;
    return ONEWIRE_SUCCESS;
}

OneWireStatus OneWire_Search(OneWire_t* OneWireStruct, uint8_t command) {
    if (!OneWireStruct) return ONEWIRE_FAILURE;
    uint8_t id_bit_number;
    uint8_t last_zero, rom_byte_number, search_result;
    uint8_t id_bit, cmp_id_bit;
    uint8_t rom_byte_mask, search_direction;

    /* Initialize for search */
    id_bit_number   = 1;
    last_zero       = 0;
    rom_byte_number = 0;
    rom_byte_mask   = 1;
    search_result   = 0;

    /* Check if any devices */
    if (!OneWireStruct->LastDeviceFlag) {
        /* If presence pulse is not good after reset */
        if (OneWire_Reset(OneWireStruct)==ONEWIRE_FAILURE) {
            /* Reset the search */
            OneWireStruct->LastDiscrepancy       = 0;
            OneWireStruct->LastDeviceFlag        = 0;
            OneWireStruct->LastFamilyDiscrepancy = 0;
            return ONEWIRE_FAILURE;
        }

        /* Issue the search command */
        OneWire_WriteByte(OneWireStruct, command);  

        /* Loop to do the search */
        do {
            /* Read a bit and its complement */
            OneWire_ReadBit(OneWireStruct,&id_bit);
            OneWire_ReadBit(OneWireStruct,&cmp_id_bit);

            /* Check for no devices on 1-wire */
            if ((id_bit == 1) && (cmp_id_bit == 1)) {
                break;
            } else {
                /* All devices coupled have 0 or 1 */
                if (id_bit != cmp_id_bit) {
                    /* Bit write value for search */
                    search_direction = id_bit;
                } else {
                    /* If this discrepancy is before the Last Discrepancy on a previous next then pick the same as last time */
                    if (id_bit_number < OneWireStruct->LastDiscrepancy) {
                        search_direction = ((OneWireStruct->ROM_NUM[rom_byte_number] & rom_byte_mask) > 0);
                    } else {
                        /* If equal to last pick 1, if not then pick 0 */
                        search_direction = (id_bit_number == OneWireStruct->LastDiscrepancy);
                    }
                    
                    /* If 0 was picked then record its position in LastZero */
                    if (search_direction == 0) {
                        last_zero = id_bit_number;

                        /* Check for Last discrepancy in family */
                        if (last_zero < 9) {
                            OneWireStruct->LastFamilyDiscrepancy = last_zero;
                        }
                    }
                }

                /* Set or clear the bit in the ROM byte rom_byte_number with mask rom_byte_mask */
                if (search_direction == 1) {
                    OneWireStruct->ROM_NUM[rom_byte_number] |= rom_byte_mask;
                } else {
                    OneWireStruct->ROM_NUM[rom_byte_number] &= ~rom_byte_mask;
                }
                
                /* Serial number search direction write bit */
                OneWire_WriteBit(OneWireStruct, search_direction);

                /* Increment the byte counter id_bit_number and shift the mask rom_byte_mask */
                id_bit_number++;
                rom_byte_mask <<= 1;

                /* If the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask */
                if (rom_byte_mask == 0) {
                    rom_byte_number++;
                    rom_byte_mask = 1;
                }
            }
        /* Loop until through all ROM bytes 0-7 */
        } while (rom_byte_number < ROM_DATA_SIZE_BYTES);

        /* If the search was successful then */
        if (!(id_bit_number <= ROM_DATA_SIZE_BITS)) {
            /* Search successful so set LastDiscrepancy, LastDeviceFlag, search_result */
            OneWireStruct->LastDiscrepancy = last_zero;

            /* Check for last device */
            if (OneWireStruct->LastDiscrepancy == 0) {
                OneWireStruct->LastDeviceFlag = 1;
            }

            search_result = 1;
        }
    }

    /* If no device found then reset counters so next 'search' will be like a first */
    if (!search_result || !OneWireStruct->ROM_NUM[0]) {
        OneWireStruct->LastDiscrepancy       = 0;
        OneWireStruct->LastDeviceFlag        = 0;
        OneWireStruct->LastFamilyDiscrepancy = 0;
        search_result                        = 0;
    }

    return search_result == ONEWIRE_SUCCESS;
}

OneWireStatus OneWire_Verify(OneWire_t* OneWireStruct) {
    if (!OneWireStruct) return ONEWIRE_FAILURE;
    uint8_t rom_backup[ROM_DATA_SIZE_BYTES];
    int i,rslt,ld_backup,ldf_backup,lfd_backup;

    /* Keep a backup copy of the current state */
    for (i = 0; i < ROM_DATA_SIZE_BYTES; i++)
    rom_backup[i] = OneWireStruct->ROM_NUM[i];
    ld_backup  = OneWireStruct->LastDiscrepancy;
    ldf_backup = OneWireStruct->LastDeviceFlag;
    lfd_backup = OneWireStruct->LastFamilyDiscrepancy;

    /* Set search to find the same device */
    OneWireStruct->LastDiscrepancy = ROM_DATA_SIZE_BITS;
    OneWireStruct->LastDeviceFlag = 0;

    if (OneWire_Search(OneWireStruct, ONEWIRE_CMD_SEARCHROM)==ONEWIRE_SUCCESS) {
        /* Check if same device found */
        rslt = 1;
        for (i = 0; i < ROM_DATA_SIZE_BYTES; i++) {
            if (rom_backup[i] != OneWireStruct->ROM_NUM[i]) {
                rslt = 1;
                break;
            }
        }
    } else {
        rslt = 0;
    }

    /* Restore the search state */
    for (i = 0; i < ROM_DATA_SIZE_BYTES; i++) {
        OneWireStruct->ROM_NUM[i] = rom_backup[i];
    }
    OneWireStruct->LastDiscrepancy       = ld_backup;
    OneWireStruct->LastDeviceFlag        = ldf_backup;
    OneWireStruct->LastFamilyDiscrepancy = lfd_backup;

    /* Return the result of the verify */
    return rslt == ONEWIRE_SUCCESS;
}

OneWireStatus OneWire_TargetSetup(OneWire_t* OneWireStruct, uint8_t family_code) {
    if (!OneWireStruct) return ONEWIRE_FAILURE;
    /* Set the search state to find SearchFamily type devices */
    OneWireStruct->ROM_NUM[0] = family_code;
    for (uint8_t i = 1; i < ROM_DATA_SIZE_BYTES; i++) {
        OneWireStruct->ROM_NUM[i] = 0;
    }

    OneWireStruct->LastDiscrepancy       = ROM_DATA_SIZE_BITS;
    OneWireStruct->LastFamilyDiscrepancy = 0;
    OneWireStruct->LastDeviceFlag        = 0;
    return ONEWIRE_SUCCESS;
}

OneWireStatus OneWire_FamilySkipSetup(OneWire_t* OneWireStruct) {
    if (!OneWireStruct) return ONEWIRE_FAILURE;
    /* Set the Last discrepancy to last family discrepancy */
    OneWireStruct->LastDiscrepancy = OneWireStruct->LastFamilyDiscrepancy;
    OneWireStruct->LastFamilyDiscrepancy = 0;

    /* Check for end of list */
    if (OneWireStruct->LastDiscrepancy == 0) {
        OneWireStruct->LastDeviceFlag = 1;
    }
    return ONEWIRE_SUCCESS;
}

OneWireStatus OneWire_GetROM(OneWire_t* OneWireStruct, uint8_t index, uint8_t* ROM) {
    if (!OneWireStruct || !ROM) return ONEWIRE_FAILURE;
    *ROM = OneWireStruct->ROM_NUM[index];
    return ONEWIRE_SUCCESS;
}

OneWireStatus OneWire_GetFullROM(OneWire_t* OneWireStruct, uint8_t *firstIndex) {
    if (!OneWireStruct || !firstIndex) return ONEWIRE_FAILURE;
    uint8_t i;
    for (i = 0; i < ROM_DATA_SIZE_BYTES; i++) {
        *(firstIndex + i) = OneWireStruct->ROM_NUM[i];
    }
    return ONEWIRE_SUCCESS;
}

OneWireStatus OneWire_Select(OneWire_t* OneWireStruct, uint8_t* addr) {
    if (!OneWireStruct || !addr) return ONEWIRE_FAILURE;
    uint8_t i;
    OneWire_WriteByte(OneWireStruct, ONEWIRE_CMD_MATCHROM);
    
    for (i = 0; i < ROM_DATA_SIZE_BYTES; i++) {
        OneWire_WriteByte(OneWireStruct, addr[i]);
    }
    return ONEWIRE_SUCCESS;
}

OneWireStatus OneWire_SelectWithPointer(OneWire_t* OneWireStruct, uint8_t* ROM) {
    if (!OneWireStruct || !ROM) return ONEWIRE_FAILURE;
    OneWire_WriteByte(OneWireStruct, ONEWIRE_CMD_MATCHROM);
    for (uint8_t i = 0; i < ROM_DATA_SIZE_BYTES; i++) {
        OneWire_WriteByte(OneWireStruct, ROM[i]);
    }   
    return ONEWIRE_SUCCESS;
}

OneWireStatus OneWire_CRC8(uint8_t *addr, uint8_t len, uint8_t* rslt) {
    if (!addr) return ONEWIRE_FAILURE;
    uint8_t crc = 0, inbyte, i, mix;
    while (len--) {
        inbyte = *addr++;
        for (i = ROM_DATA_SIZE_BYTES; i; i--) {
            mix = (crc ^ inbyte) & 0x01;
            crc >>= 1;
            if (mix) {
                crc ^= 0x8C;
            }
            inbyte >>= 1;
        }
    }
    
    /* Return calculated CRC */
    *rslt = crc;
    return ONEWIRE_SUCCESS;
}
