#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "http_imp.h"

#include <esp_log.h>
#include <esp_timer.h>

#include <esp_http_server.h>

#include "freertos/FreeRTOS.h"
#include "nvs.h" // for saving the wifi credentials
#include "html_page.h" // for the html page to be sent to the client
#include "wifi-imp.h" 

static const char* TAG = "HTTP";


static void url_decode(char *dst, const char *src)
{
    while (*src) {
        if (*src == '%' &&
            src[1] != '\0' &&
            src[2] != '\0' &&
            isxdigit((unsigned char)src[1]) &&
            isxdigit((unsigned char)src[2])) {

            char hex[3] = { src[1], src[2], '\0' };
            *dst++ = (char)strtol(hex, NULL, 16);
            src += 3;
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else {
            *dst++ = *src++;
        }
    }

    *dst = '\0';
}

// {
//     // purpose: Browser can send some special characters directly inside the HTTP request
//     /*
//         Example :
//         Space -> +
//         @ -> %40
//         / -> %2F

//         This function will convert the encoded text back into normal

//         Example:
//         Input : "My_+Wifi%40123"
//         Output: "My Wifi@123"
//     */

//     char a, b; // Temporary variables used to hold the 2 hexadecimal characters
//     while(*src)
//     {
//         // loop through the entire source string
//         if((*src == '%') && (a == src[1] && b == src[2]) && isxdigit(a) && isxdigit(b)) // check if the current character is "%" and ensure the next two characters exist
//         // and ensure both of them are valid hexadecimal digits
//         {
//             if (a >= 'a')
//             {
//                 a -= 'a' - 'A'; // convert lower case hex letters to uppercase
//             }
//             if (a >= 'A')
//             {
//                 /*
//                 'A' - ASCII = 65
//                 We want
//                 'A' -> 10

//                 So :
//                     65 - 55 = 10
//                 */
//                 a -= ('A' - 10);
//             }
//             else
//             {
//                 // Convert ASCII digits to numeric values
//                 /*
//                     Example:
//                     '4' ASCII = 52
//                     '0' ASCII = 48
//                     52 - 48 = 4
//                 */
//                 a -= '0'; 
//             }

//             // Do the same for 'b'
//             if (b >= 'a')
//             {
//                 b -= 'a' - 'A';
//             }
//             if (b >= 'A')
//             {
//                 b -= ('A' - 10);
//             }
//             else
//             {
//                 b -= '0';
//             }
//             /*
//             Combine both hexadecimal digit into 1 byte
//             Formula:
//                  (first_digit * 16) + second_digit
//                  Example : %40
//                  a = 4
//                  b = 0

//                  4*16+0 = 64 

//                  ASCII 65 = @
//                  so '%40" becomes '@'
//             */
//            *dst++ = 16 * a + b;
//            src += 3; // skip past the 3 characters we have processed '%' '4' '0'
//         }
//         //handle space
//         else if (*src == '+')
//         {
//             *dst++ = ' '; // convert '+' to space
//             src++;
//         }
//         // Normal character, just copy it as is
//         else
//         {
//             *dst++ = *src++; // copy the character as is
//         }
//     }
//     *dst++ = '\0'; // null terminate the destination string
// }

// static void parse_form(char* body, char* ssid, char* pass, char* http_user, char* http_pass)
// {
//     // the request will be something of the type ssid=ssid&password=password&...
//     char* token = strtok(body, "&");
//     while (token)
//     {
//         /*code*/
//         char key[32] = {0};
//         char value[64] = {0};

//         // split the token into key and value
//         // Example : token = "ssid=HomeWiFi"
//         // result: key = "ssid", value = "HomeWiFi"
//         sscanf(token, "%31[^=]=%63s", key, value);

//         char decode[64] = {0}; //buffer to hold the decoded value

//         // need to decode the request coming from browser
//         url_decode(decode, value);

//         if (strcmp(key, "ssid") == 0)
//         {
//             strncpy(ssid, decoded, 31); // copy ssid into the ssid array
//         }
//         if (strcmp(key, "password") == 0)
//         {
//             strncpy(pass, decoded, 63); // copy password into the pass array
//         }
//         if (strcmp(key, "http_user") == 0)
//         {
//             strncpy(http_user, decoded, 31); // copy http_user into the http_user array
//         }
//         if (strcmp(key, "http_pass") == 0)
//         {
//             strncpy(http_pass, decoded, 63); // copy http_pass into the http_pass array
//         }
//     }
//     token = strtok(NULL, "&"); // move to the next token separated by "&"
// }

static void parse_form(char *body,
                       char *ssid,
                       char *pass,
                       char *http_user,
                       char *http_pass)
{
    char *token = strtok(body, "&");

    while (token != NULL) {
        char *equals = strchr(token, '=');

        if (equals != NULL) {
            *equals = '\0';

            const char *key = token;
            const char *value = equals + 1;
            char decoded[128] = {0};

            url_decode(decoded, value);

            if (strcmp(key, "ssid") == 0) {
                strncpy(ssid, decoded, 31);
                ssid[31] = '\0';
            } else if (strcmp(key, "password") == 0) {
                strncpy(pass, decoded, 63);
                pass[63] = '\0';
            } else if (strcmp(key, "http_user") == 0) {
                strncpy(http_user, decoded, 63);
                http_user[63] = '\0';
            } else if (strcmp(key, "http_pass") == 0) {
                strncpy(http_pass, decoded, 63);
                http_pass[63] = '\0';
            }
        }

        token = strtok(NULL, "&");
    }
}
static esp_err_t root_get_handler(httpd_req_t* req)
{
    // Sending the wifi credentials form to the client
    httpd_resp_send(req, wifi_form_html, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static void save_http_credentials(const char* user, const char* pass)
{
    nvs_handle nvs;
    esp_err_t ret = nvs_open("http", NVS_READWRITE, &nvs);
    if (ret == ESP_OK)
    {
        nvs_set_str(nvs, "user", user);
        nvs_set_str(nvs, "pass", pass);

        nvs_commit(nvs);
        nvs_close(nvs);

        ESP_LOGI(TAG, "HTTP credentials saved successfully in NVS");
    }
    else
    {
        ESP_LOGE(TAG, "HTTP credentials Failed to save");
    }
}

static esp_err_t wifi_post_handler(httpd_req_t* req)
{
    char buf[256] = {0};
    int len = httpd_req_recv(req, buf, sizeof(buf) - 1);

    if (len <= 0)
    {
        return ESP_FAIL;
    }
    char ssid[32] = {0};
    char pass[64] = {0};
    char http_user[64] = {0};
    char http_pass[64] = {0};

    ESP_LOGI(TAG, "Received Wifi credentials form submission");

    // We need to parse the form to extract the exact fields from the request
    parse_form(buf, ssid, pass, http_user, http_pass);

    if (strlen(ssid) == 0 || strlen(pass) == 0 || strlen(http_user) == 0 || strlen(http_pass) == 0)
    {
        ESP_LOGE(TAG, "Missing Required Fields in the form submission");
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing Required Fields");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Provisioning Data validated, Saving credentials to NVS");
    ESP_LOGI(TAG, "SSID : %s", ssid);
    ESP_LOGI(TAG, "Password : %s", pass);
    ESP_LOGI(TAG, "HTTP Username : %s", http_user);
    ESP_LOGI(TAG, "HTTP Password : %s", http_pass);

    save_wifi_credentials(ssid, pass); // save wifi credentials to NVS
    // We also need to save HTTP credentials to NVS
    save_http_credentials(http_user, http_pass);

    httpd_resp_sendstr(req, "Saved, Rebooting...");

    vTaskDelay(pdMS_TO_TICKS(2000)); // wait for 2 seconds before rebooting
    esp_restart(); // reboot the device to apply the new credentials

    return ESP_OK;

}

void http_wifi_credentials_server_init(void)
{
    httpd_config_t config_wifi_server = HTTPD_DEFAULT_CONFIG(); // Default configurations for the wifi credentials server
    httpd_handle_t wifi_server = NULL; // http server handle

    if (httpd_start(&wifi_server, &config_wifi_server) == ESP_OK)
    {
        ESP_LOGI(TAG, "WIFI credentials http server started successfully");
        // Register URI Handler for wifi-credentials

        static httpd_uri_t root = {
            .uri ="/",
            .method = HTTP_GET,
            .handler = root_get_handler, // need to declare this yet
        };
        static httpd_uri_t wifi = { // Handle post request coming to "/wifi"
            .uri ="/wifi",
            .method = HTTP_POST,
            .handler = wifi_post_handler, // need to declare this yet
        };

        httpd_register_uri_handler(wifi_server, &root);
        httpd_register_uri_handler(wifi_server, &wifi);

        ESP_LOGI(TAG, "HTTP server started successfully");
    }
    else
    {
        ESP_LOGE(TAG, "Failed to start HTTP server");
    }
}
