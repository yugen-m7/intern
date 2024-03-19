#include "esp_event_base.h"
#include "freertos/projdefs.h"
#include <stdio.h>
#include <connect.h>
#include <esp_log.h>
#include <mqtt_client.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char* TAG_M = "MQTT_EVENT";

static void mqtt_event_handler(void *event_handler_arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
  esp_mqtt_event_handle_t event = event_data;
  esp_mqtt_client_handle_t client = event->client;
  switch(event->event_id){
    case MQTT_EVENT_BEFORE_CONNECT:
      ESP_LOGI(TAG_M, "MQTT_EVENT_BEFORE_CONNECT");
      break; 
    case MQTT_EVENT_CONNECTED:
      ESP_LOGI(TAG_M, "MQTT_EVENT_CONNECTED");
      esp_mqtt_client_subscribe(client, "void", 2);
      esp_mqtt_client_subscribe(client, "ion", 2);
      esp_mqtt_client_subscribe(client, "luke", 2);
      break;
    case MQTT_EVENT_DISCONNECTED:
      ESP_LOGI(TAG_M, "MQTT_EVENT_DISCONNECTED");
      break;
    case MQTT_EVENT_SUBSCRIBED:
      ESP_LOGI(TAG_M, "MQTT_EVENT_SUBCRIBED");
      break;
    case MQTT_EVENT_UNSUBSCRIBED:
      ESP_LOGI(TAG_M, "MQTT_EVENT_UNSUBSCRIBED");
      break;
    case MQTT_EVENT_DATA:
      ESP_LOGI(TAG_M, "MQTT_EVENT_DATA");
        printf("%.*s: ", event->topic_len, event->topic);
        printf("%.*s\n", event->data_len, event->data);
      break;
    case MQTT_EVENT_ERROR:
      ESP_LOGE(TAG_M, "MQTT_EVENT_ERROR");
      break;
    default:
      break;
  }

}

void mqtt_init(){

  esp_mqtt_client_config_t mqtt_cfg={
        .broker.address.uri = "mqtt://mqtt.eclipseprojects.io",
  };
  esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
  esp_mqtt_client_start(client);
}

void app_main(void)
{
  printf("wifi blaw blaw blaw\n");
  wifiInit();
  wifiConnect();


  vTaskDelay(pdMS_TO_TICKS(2000));
  
  mqtt_init();

}
