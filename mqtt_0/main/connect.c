#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <nvs_flash.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_netif.h>
#include <esp_log.h>

static const char *TAG = "WIFI";

esp_netif_t *esp_netif;

void event_handler(void *event_handler_arg, esp_event_base_t event_base,
                   int32_t event_id, void *event_data){
  switch(event_id){
    case WIFI_EVENT_STA_START:
      ESP_LOGI(TAG, "CONNECTING.......");
      esp_wifi_connect();
    break;
    case WIFI_EVENT_STA_CONNECTED:
      ESP_LOGI(TAG, "CONNECTED");
    break;
    case WIFI_EVENT_STA_DISCONNECTED:
      ESP_LOGI(TAG, "DICONNECTED");
    break;
    case IP_EVENT_STA_GOT_IP:
      ESP_LOGI(TAG, "IP_EVENT_STA_GOT_IP");
    break;
  default:
    break;
  }

}

void wifiInit(){
  nvs_flash_init();
  esp_netif_init();
  esp_event_loop_create_default();
  esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, event_handler, NULL);
  esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, event_handler, NULL);
  wifi_init_config_t wifi_init = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&wifi_init);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
}

void wifiConnect(){
  esp_netif = esp_netif_create_default_wifi_sta();
  esp_wifi_set_mode(WIFI_MODE_STA);

  wifi_config_t wifi_config={
    .sta.ssid = "nepaldigisys",
    .sta.password = "NDS_0ffice",
  };
  esp_wifi_set_config(WIFI_IF_STA, &wifi_config);

  esp_wifi_start();
}
