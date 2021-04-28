
#include "temp_sensor.h"

/**
 * @defgroup TEMP_SENSOR Functions
 * @brief    Library Functions
 * @{
 */

/**
 * @brief  Assign the 
 * @param  time_us: time to put system to sleep
 * @return retval: see defintion of OneWireStatus
 */

temp_sensor_status_t temp_sensor_registration(temp_sensor_t* temp_sensor, 
                                              temp_sensor_status_t (*fp_init)(temp_config_t* temp_config), 
                                              temp_sensor_status_t (*fp_read)(temp_config_t* temp_config, float* temp)){
    if ( !temp_sensor ||!fp_init || !fp_read) return TEMP_SENSOR_USAGE_ERROR;
    temp_sensor->fp_init = fp_init;
    temp_sensor->fp_read = fp_read;
    return TEMP_SENSOR_SUCCESS;
}