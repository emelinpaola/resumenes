#include <string.h> 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "esp_http_server.h"  // Requiere habilitar "HTTP Server" en menuconfig

#define LED_GPIO GPIO_NUM_2

static const char *TAG = "wifi softAP";

esp_err_t led_handler(httpd_req_t *req) {
    char buf[10];
    int ret = httpd_req_recv(req, buf, sizeof(buf));
    if (ret <= 0) return ESP_FAIL;

    buf[ret] = 0;

    if (strstr(buf, "ON")) {
        gpio_set_level(LED_GPIO, 1);
    } else if (strstr(buf, "OFF")) {
        gpio_set_level(LED_GPIO, 0);
    }

    const char* resp = "OK";
    httpd_resp_send(req, resp, strlen(resp));
    return ESP_OK;
}

esp_err_t root_get_handler(httpd_req_t *req) {
    const char* html =
        "<!DOCTYPE html><html><head><title>LED Control</title>"
        "<style>"
        "body { background-color: #f0f8ff; font-family: 'Segoe UI', sans-serif; text-align: center; padding-top: 50px; }"
        "h1 { color: #003366; font-size: 2em; margin-bottom: 30px; }"
        "button { background-color: #1e90ff; color: white; font-size: 1.2em; padding: 15px 30px; "
        "margin: 10px; border: none; border-radius: 8px; cursor: pointer; transition: 0.3s; }"
        "button:hover { background-color: #005fbb; }"
        "</style></head><body>"
        "<h1>✨ Controla el LED ✨</h1>"
        "<button onclick=\"fetch('/led', {method:'POST', body:'ON'})\">ENCENDER</button>"
        "<button onclick=\"fetch('/led', {method:'POST', body:'OFF'})\">APAGAR</button>"
        "</body></html>";

    httpd_resp_send(req, html, strlen(html));
    return ESP_OK;
}

httpd_handle_t start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t root_uri = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = root_get_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &root_uri);

        httpd_uri_t led_uri = {
            .uri = "/led",
            .method = HTTP_POST,
            .handler = led_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &led_uri);
    }

    return server;
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data) {
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d", MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d", MAC2STR(event->mac), event->aid);
    }
}

void wifi_init_softap(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = "emelin",
            .ssid_len = strlen("emelin"),
            .channel = 1,
            .password = "123456789",
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {.required = true},
        },
    };

    if (strlen("123456789") == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "SoftAP iniciado. SSID: %s", "emelin");
}

void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    ESP_LOGI(TAG, "Iniciando WiFi...");
    wifi_init_softap();

    ESP_LOGI(TAG, "Iniciando servidor web...");
    start_webserver();
}
