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
    /* Other Temp Sensor Setup goes here */
} temp_config_t;

typedef struct {

    /* pointer to generic init function that return 1 on success 0 on failure,
    takes in a function whose parameter is temp_sensor_t, which will hold
    all information regarding the sensor. */  
    temp_sensor_status_t (*fp_init)(temp_config_t* temp_config);   

    /* pointer to generic read function that return 1 on success 0 on failure,
    takes in a function whose parameter is temp_sensor_t, which will hold
    all relevant information regarding the sensor. */  
    temp_sensor_status_t (*fp_read)(temp_config_t* temp_config, float* temp);

} temp_sensor_t;

/* initialize default values */

temp_sensor_status_t temp_sensor_registration(temp_sensor_t* temp_sensor, 
                                              temp_sensor_status_t (*fp_init)(temp_config_t* temp_config), 
                                              temp_sensor_status_t (*fp_read)(temp_config_t* temp_config, float* temp));

#endif /* TEMP_SENSOR_H */

