#include "esp_event_base.h"
#include "freertos/projdefs.h"
#include <stdio.h>
#include <connect.h>
#include <esp_log.h>
#include <mqtt_client.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <dht.h>

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
      esp_mqtt_client_subscribe(client, "dht", 2);
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
    case MQTT_EVENT_PUBLISHED:
      ESP_LOGI(TAG_M, "MQTT_EVENT_PUBLISHED");
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


static void mqtt_publish(void* args)
{
    // uint8_t* data = 0;
    // char* message=(char*)(&humidity);
    // char* message="hello world";
    char data[16];
    esp_mqtt_client_handle_t client = ((esp_mqtt_client_handle_t)args);
    while (true)
    {
        snprintf(data, sizeof(data), "%.2f\t%.2f\n",get_humidity(),get_temperature());
        esp_mqtt_client_publish(client, "esp", data, sizeof(data), 1, 0);
        // printf("%s\n",data);
        // data++;
        vTaskDelay(pdMS_TO_TICKS(4000));
    }
}

static void mqtt_init(){
  esp_mqtt_client_config_t mqtt_cfg={
        .broker.address.uri = "mqtt://mqtt.eclipseprojects.io",
  };
  esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
  esp_mqtt_client_start(client);

  xTaskCreate(mqtt_publish, "publish",2048,(void*)client, 2, NULL);
}

void app_main(void)
{
  wifiInit();
  wifiConnect();
  //
  vTaskDelay(pdMS_TO_TICKS(2000));
  //
  mqtt_init();
  dht_init();
}
