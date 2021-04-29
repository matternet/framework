#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H

#include <stdbool.h>
#include <stdint.h>

// forward declaration of OneWire_t will use as abstraction
typedef struct OneWire_t OneWire_t;

/* General Temp Sensor return codes */
typedef enum {
    TEMP_SENSOR_USAGE_ERROR             =  -4, // Error: Usage Error.
    TEMP_SENSOR_FAILURE                 =  -3, // Error: Operation Failure.
    TEMP_SENSOR_TIMEOUT                 =  -2, // Error: Operation Timeout.
    TEMP_SENSOR_CONVERSION_IN_PROGRESS  =  -1, // Temp sensor still processing information
    TEMP_SENSOR_SUCCESS                 =   0  // Operation Successful
} temp_sensor_status_t;

// hold onto all information needed for sensors
typedef struct temp_config_t {
    /* Needed for DS18B20 Setup */
    OneWire_t* p_one_wire_struct;
    uint32_t   one_wire_pal_line;
    /* Other Temp Sensor params goes here */
} temp_config_t;

typedef struct {

    /**
     * @brief  function pointer to an init function for a temperature sensor
     * @param  temp_config_t*: pointer to a config struct, should hold any necessary information about the sensor
     * @return retval: see defintion of OneWireStatus
     */
    temp_sensor_status_t (*fp_init)(temp_config_t* temp_config);   

    /**
     * @brief  function pointer to a read function for a temperature sensor
     * @param  temp_config_t*: pointer to a config struct, should hold any necssary information about the sensor
     * @param  float*:         pointer to a float, temperature value read will be stored in here. 
     * @return retval: see defintion of OneWireStatus
     */
    temp_sensor_status_t (*fp_read)(temp_config_t* temp_config, float* temp);

    /* Eacb temperature sensor must have a config struct properly defined per each sensor*/
    temp_config_t* temp_config;

} temp_sensor_t;

/* initialize default values */

temp_sensor_status_t register_temp_sensor(temp_sensor_t* temp_sensor, 
                                              temp_sensor_status_t (*fp_init)(temp_config_t* temp_config), 
                                              temp_sensor_status_t (*fp_read)(temp_config_t* temp_config, float* temp));

#endif /* TEMP_SENSOR_H */

