#include "app_state.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "stdio.h"

static app_state_t current_state= APP_STATE_INIT;

static SemaphoreHandle_t state_mutex;// state mutex for securing the enum 

void app_state_init(void)
{
    state_mutex = xSemaphoreCreateMutex();
    if (state_mutex != NULL) {
        printf("State mutex created successfully\n");
    }
}

void app_set_state(app_state_t new_state){
    if(state_mutex)
    {
        xSemaphoreTake(state_mutex, portMAX_DELAY);
        current_state = new_state;
        xSemaphoreGive(state_mutex);
    }
}

app_state_t app_get_state(void)
{
    app_state_t state;

    if (state_mutex)
    {
        xSemaphoreTake(state_mutex, portMAX_DELAY);
        state = current_state;
        xSemaphoreGive(state_mutex);
        return state;

    }

    return APP_STATE_RESTART;
}