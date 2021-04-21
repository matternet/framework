#include "temp_sensor.h"

void TempSensor_registration( void (*init)(void), uint16_t(*read)(void)){
    myTemp.init = init;
    myTemp.read = read;
}