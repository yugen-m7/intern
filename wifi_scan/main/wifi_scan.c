#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include <esp_wifi.h>
#include <esp_log.h>

#define MAX_AP 10

void wifi_init(){

    ESP_ERROR_CHECK_WITHOUT_ABORT(nvs_flash_init());
    esp_netif_init();
    esp_event_loop_create_default();

    wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_init(&wifi_config));
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_start());
}

void app_main(void){
    wifi_init();

    wifi_scan_config_t scan_config ={
        .bssid =0,
        .ssid = 0,
        .channel = 0,
    };
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_scan_start(&scan_config, true));

    wifi_ap_record_t wifi_records[MAX_AP];
    uint16_t max_records = MAX_AP;
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_scan_get_ap_records(&max_records , wifi_records));

    for(int i=0 ; i< max_records ; i++){
        printf("%s\n",(char*) wifi_records[i].ssid);
    }
}
