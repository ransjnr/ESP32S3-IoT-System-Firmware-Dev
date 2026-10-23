#ifndef WIFI_IMP_H
#define WIFI_IMP_H

void wifi_init(void);

void wifi_init_station(void);

esp_err_t save_wifi_credentials(const char* ssid, const char* pass);

void start_ap_mode(void);


#endif
