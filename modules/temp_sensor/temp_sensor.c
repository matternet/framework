/** 
 * @file temp_sensor.c
 *
 * @brief abstraction module for temperature sensors.
 *
 * @copyright Copyright © 2020 Matternet. All rights reserved.
 *
 */

#include "temp_sensor.h"

#ifdef UNIT_TEST
#include "timing.h"
#else
#include <modules/timing/timing.h>
#include <modules/uavcan_debug/uavcan_debug.h>
#endif // UNIT_TEST

/**
 * @defgroup TEMP_SENSOR Functions
 * @brief    Library Functions
 * @{
 */

/**
 * @brief  Assign init and read functions to abstract temperature sensor
 * @param  temp_sensor: pointer to a temp_sensor_t
 * @param  fp_init: function pointer to a init function for a particular temp sensor.
 * @param  fp_read: function pointer to a read function for a particular temp sensor.
 * @return retval: see defintion of temp_sensor_status_t
 */
temp_sensor_status_t register_temp_sensor(temp_sensor_t* temp_sensor, 
                                              temp_sensor_status_t (*fp_init)(temp_config_t* temp_config), 
                                              temp_sensor_status_t (*fp_read)(temp_config_t* temp_config, float* temp)){
    if ( !temp_sensor || !fp_init || !fp_read) {
        return TEMP_SENSOR_USAGE_ERROR;
    } 

    temp_sensor->fp_init = fp_init;
    temp_sensor->fp_read = fp_read;
    
    return TEMP_SENSOR_SUCCESS;
}

/**
 * @brief  Wrapper init function to be used with temp_config_t struct;
 * @param  temp_config: pointer to a temp_config_t with proper onewire parameters
 * @return retval: see defintion of temp_sensor_status_t
 */
temp_sensor_status_t OneWire_System_Init(temp_config_t* temp_config){
    if (!(temp_config) || !(temp_config->p_one_wire_struct)) {
        return TEMP_SENSOR_USAGE_ERROR;
    }
    /* set the pal line, try to get the ROM */
    OneWire_Init(temp_config->p_one_wire_struct, temp_config->one_wire_pal_line);
    if (OneWire_First(temp_config->p_one_wire_struct) == ONEWIRE_SUCCESS){
        return TEMP_SENSOR_SUCCESS;
    }
    return TEMP_SENSOR_FAILURE;
}

/**
 * @brief  Starts the conversion process AND modifies value of p_temp_degC with measured temperature.
 * @param  *temp_config: address of temp_config struct, with an initialized onewire struct
 * @param  *p_temp_degC: address of float to store measured temperature
 * @return retval: see defintion of temp_sensor_status_t
 */
temp_sensor_status_t DS18B20_Wrapper_Read(temp_config_t* temp_config, float* p_temp_degC) {
    if (!p_temp_degC                      ||
        !temp_config                      ||
        !temp_config->p_one_wire_struct   ||
        !temp_config->p_one_wire_struct->ROM_NUM) {
         return TEMP_SENSOR_USAGE_ERROR;
    }

    /* Begin the conversion */
    DS18B20_StartAll(temp_config->p_one_wire_struct);
    uint32_t ts_start_ms = millis();
    
    /* Read the line until DS18B20 pulls low, signaling the data is ready. 
       Works more consistently than DS18B20_AllDone. Likely due to 
       1 less read needed and the time cost associated with that read */
    DS18B20_Status read_status;
    do {
        if ( millis() > (uint32_t) ts_start_ms + DS18B20_MAX_CONVERSION_TIME_MS ) {
            /* Presence pulse not detected in time */
            return TEMP_SENSOR_TIMEOUT;
        }
        read_status = DS18B20_Read(temp_config->p_one_wire_struct, 
                                   temp_config->p_one_wire_struct->ROM_NUM, 
                                   p_temp_degC);
    } while (read_status == DS18B20_CONVERSION_IN_PROGRESS);
    
    return (read_status == DS18B20_SUCCESS) ? TEMP_SENSOR_SUCCESS : TEMP_SENSOR_FAILURE;
}

/**
 * @brief  Perform a temperature sensor read, update the fields for an env_status message 
 * @param  temp_sensor_t*: pointer to a sensor struct, should hold any necssary information about the sensor
 * @param  env_status    : pointer to an env_status struct. Commonly used for publishing.
 * @return retval:         None
 */
void temp_sensor_update_fields_for_env_status_msg(temp_sensor_t* temp_sensor, struct com_matternet_equipment_env_EnvStatus_s* env_status){
    uavcan_send_debug_msg(UAVCAN_PROTOCOL_DEBUG_LOGLEVEL_INFO, "temp_sensor_update_fields_for_env_status_msg", "");    
    /* initialize to some obviously wrong value, retains last value for easier plotting, and is likely the scenario since temperature does not fluctuate rapidly */
    static float temp_degC = -200.0;
    static temp_sensor_status_t read_error = TEMP_SENSOR_FAILURE;

    read_error = temp_sensor->fp_read(temp_sensor->temp_config, &temp_degC);
    env_status->temp_degC = temp_degC;
    env_status->temp_sensor_read_error = (read_error!=TEMP_SENSOR_SUCCESS);
    if (env_status->temp_sensor_read_error)
        env_status->temp_sensor_total_errors++;
}