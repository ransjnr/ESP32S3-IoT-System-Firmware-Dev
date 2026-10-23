#ifndef TELEMETRY_H_
#define TELEMETRY_H_

#include "stdint.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_err.h"

typedef struct{
    int8_t temperature;
    uint8_t humidity;
}sensor_data_t;

esp_err_t telemetry_init(void);
BaseType_t telemetry_enque(int8_t temperature, uint8_t humidity);
BaseType_t telemetry_deuque(sensor_data_t* out, TickType_t tick);

#endif