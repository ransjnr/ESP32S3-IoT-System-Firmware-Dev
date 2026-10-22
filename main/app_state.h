#ifndef APP_STATE_H
#define APP_STATE_H

typedef enum {
    APP_STATE_INIT = 0,
    APP_STATE_WIFI_CONNECT,
    APP_STATE_TELEMETRY_INIT,
    APP_STATE_RUNNING,
    APP_STATE_RESTART,
    APP_STATE_ERROR
} app_state_t;

/*
 * @brief This function will basically initialize the state machine
*/
void app_state_init(void);

/**
 * @brief  this function will basically set the state of the main state machine
 */
void app_set_state(app_state_t new_state);

/**
 * @brief  this function will basically get the state of the main state machine
 */
app_state_t app_get_state(void);

#endif