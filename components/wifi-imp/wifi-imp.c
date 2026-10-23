#include <stdio.h>
#include "wifi-imp.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_system.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "app_state.h" // for signing the top level state machine

#include "nvs.h" // for saving the wifi credentials


static EventGroupHandle_t s_wifi_event_group;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static const char* TAG = "wifi";

#define WIFI_MAX_RETRIES_PRIV 5
#define WIFI_SSID_PRIV "IMT-MEE-20260917"
#define WIFI_PASS_PRIV "LvNNMOJk"

static int s_retry_num = 0;

static char ssid[32] = {0};
static char pass[64] = {0};

static bool load_wifi_credentials(char* ssid, size_t ssid_len, char* pass, size_t pass_len)
{
    nvs_handle nvs;

    if (nvs_open("wifi", NVS_READONLY, &nvs) != ESP_OK)
    {
        return false;
    }

    esp_err_t ret1 = nvs_get_str(nvs, "ssid", ssid, &ssid_len);
    esp_err_t ret2 = nvs_get_str(nvs, "pass", pass, &pass_len);

    nvs_close(nvs);

    return (ret1 == ESP_OK && ret2 == ESP_OK);
}

esp_err_t save_wifi_credentials(const char* ssid, const char* pass)
{
    nvs_handle_t nvs;

    esp_err_t ret = nvs_open("wifi", NVS_READWRITE, &nvs);
    if (ret == ESP_OK)
    {
        esp_err_t ret2 = nvs_set_str(nvs, "ssid", ssid);
        esp_err_t ret3 = nvs_set_str(nvs, "pass", pass);

        if (ret2 == ESP_OK && ret3 == ESP_OK)
        {
            ESP_LOGI(TAG, "WiFi credentials saved successfully");
            nvs_commit(nvs);
            nvs_close(nvs);
        }
        else
        {
            ESP_LOGW(TAG, "Unable to save wifi credentials to NVS");
            nvs_close(nvs);
            return ret2;
        }
    }
    else
    {
        ESP_LOGW(TAG, "Unable to open NVS : %d", ret);
        return ret;
    }

    return ESP_OK;
}

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START){
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {    
        if (s_retry_num < WIFI_MAX_RETRIES_PRIV) 
        {
            s_retry_num++;
            ESP_LOGI(TAG, "Retry %d / %d", s_retry_num, WIFI_MAX_RETRIES_PRIV);
            esp_wifi_connect();
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }

        ESP_LOGI(TAG, "Connection to AP Failed");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP){
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        app_set_state(APP_STATE_TELEMETRY_INIT);
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_LOGI(TAG, "Wifi setting initialized");

    bool creds_loaded = load_wifi_credentials(ssid, sizeof(ssid), pass, sizeof(pass));

    if (creds_loaded)
    {
        // start in station mode
        ESP_LOGI(TAG, "Wifi Credentials Loaded from NVS");
        app_set_state(APP_STATE_WIFI_FROM_NVS);
    }
    else
    {
        // start in AP mode
        ESP_LOGW(TAG, "No Credentials found. Starting in AP mode");
        app_set_state(APP_STATE_WIFI_PROVISIONING);
    }
}

void start_ap_mode(void)
{
    esp_netif_create_default_wifi_ap();
    esp_wifi_restore(); //clear STA config to avoid auto connecting with empty credentials

    wifi_config_t ap_config = {
        .ap = {
            .ssid = "ESP32_Setup",
            .ssid_len = strlen("ESP32_Setup"),
            .password = "12345678",
            .max_connection = 2,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        }
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void wifi_init_station(void)
{



    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .failure_retry_cnt = WIFI_MAX_RETRIES_PRIV,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
            .ssid = ssid,
            .password = pass
        }
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
}