#include "esp_random.h"
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <mqtt_client.h>
#include <connect.h>
#include <esp_log.h>
#include <esp_wifi.h>

static const char* tag = "MQTT_EVENT";
const uint32_t WIFI_CONNEECTED = BIT1;
const uint32_t MQTT_CONNECTED = BIT2;
const uint32_t MQTT_PUBLISHED = BIT3;

QueueHandle_t queue_handler;
TaskHandle_t task_handler;


void mqtt_events(void* arg, esp_event_base_t event_base, uint32_t event_id, void* event_data){
  esp_mqtt_event_handle_t data = event_data;
  switch(data->event_id){
    case MQTT_EVENT_CONNECTED:
      ESP_LOGI(tag,"MQTT_EVENT_CONNECTED");
      xTaskNotify(task_handler, MQTT_CONNECTED, eSetValueWithOverwrite);
      break;
    case MQTT_EVENT_DISCONNECTED:
      ESP_LOGI(tag,"MQTT_EVENT_DISCONNECTED");
      break;
    case MQTT_EVENT_SUBSCRIBED:
      ESP_LOGI(tag,"MQTT_EVENT_SUBSCRIBED");
      break;
    case MQTT_EVENT_UNSUBSCRIBED:
      ESP_LOGI(tag,"MQTT_EVENT_UNSUBSCRIBED");
      break;
    case MQTT_EVENT_PUBLISHED:
      ESP_LOGI(tag,"MQTT_EVENT_PUBLISHED");
      xTaskNotify(task_handler, MQTT_PUBLISHED, eSetValueWithOverwrite);
      break;
    case MQTT_EVENT_DATA:
      ESP_LOGI(tag,"MQTT_EVENT_DATA");
      printf("data: %.*s\r\n",data->data_len, data->data);
      printf("topic: %.*s\r\n",data->topic_len, data->topic);
      break;
    case MQTT_EVENT_ERROR:
      ESP_LOGI(tag,"MQTT_EVENT_ERROR");
      break;
    default:
      break;
  }
}

void MQTTLogic(int sensorReading)
{
  uint32_t command = 0;
  // esp_mqtt_client_config_t mqttConfig = { .uri = "mqtt://mqtt.eclipse.org:1883"};
  esp_mqtt_client_handle_t client = NULL;

  while (true)
  {
    xTaskNotifyWait(0, 0, &command, portMAX_DELAY);
    switch (command)
    {
    case WIFI_CONNEECTED:
      // client = esp_mqtt_client_init(&mqttConfig);
      esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
      esp_mqtt_client_start(client);
      break;
    case MQTT_CONNECTED:
      esp_mqtt_client_subscribe(client, "/topic/my/subscription/1", 2);
      char data[50];
      sprintf(data, "%d", sensorReading);
      printf("sending data: %d", sensorReading);
      esp_mqtt_client_publish(client, "topic/my/publication/1", data, strlen(data), 2, false);
      break;
    case MQTT_PUBLISHED:
      esp_mqtt_client_stop(client);
      esp_mqtt_client_destroy(client);
      esp_wifi_stop();
      return;
    default:
      break;
    }
  }
}
void sender(){
  uint8_t buffer=esp_random();
  while(1){
    xQueueSend(queue_handler, &buffer, 2000); 
    printf("sending: %d\n",buffer);
    vTaskDelay(pdMS_TO_TICKS(15000));
  }
}

void reciever(){
  int buffer;
  while(1){
    if(xQueueReceive(queue_handler, &buffer, portMAX_DELAY)){
      esp_wifi_start();
      printf("data received: %d\n", buffer);
      MQTTLogic(buffer);
    }
  }
}

void app_main(void)
{
  queue_handler  = xQueueCreate(10,sizeof(int));

  wifiInit();
  wifiConnect();

  // xTaskCreate(sender, "this is a test", 2048, NULL, 2, NULL);
  // xTaskCreate(reciever, "this is a test", 2048, NULL, 2, NULL);

}
