#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H

#include <modules/ds18b20/ds18b20.h>

typedef struct {

    /* pointer to generic init function that return 1 on success 0 on failure,
    takes in a function whose parameter is temp_sensor_t, which will hold
    all information regarding the sensor. */  
    bool (*p_init)(void);   

    /* pointer to generic read function that return 1 on success 0 on failure,
    takes in a function whose parameter is temp_sensor_t, which will hold
    all relevant information regarding the sensor. */  
    bool (*p_read)(*float); 

    OneWire_t* p_onewire_struct;
    uint32_t   onewire_pal_line;

} temp_sensor_t;

/* initialize default values */
static temp_sensor_t temp_sensor = {
    .p_onewire_struct = 0;
    .onewire_pal_line = 0;
}

bool temp_sensor_registration( bool (*init)(temp_sensor_t), uint16_t(*read)(*float));

#endif /* TEMP_SENSOR_H */

