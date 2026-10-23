#ifndef HTML_PAGE_H
#define HTML_PAGE_H

static const char *wifi_form_html =
    "<!DOCTYPE html><html><body>"
    "<h1>ESP32 WiFi Setup</h1>"
    "<form method=\"POST\" action=\"/wifi\">"
    "SSID: <br><input type=\"text\" name=\"ssid\"><br>"
    "Password: <br><input type=\"password\" name=\"password\"><br><br>"
    "<h3>HTTP Credentials</h3>"
    "Username: <br><input type=\"text\" name=\"http_user\"><br>"
    "Password: <br><input type=\"password\" name=\"http_pass\"><br><br>"
    "<input type=\"submit\" value=\"Save\">"
    "</form>"
    "</body></html>";

#endif