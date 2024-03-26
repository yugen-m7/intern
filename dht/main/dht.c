#include "esp_attr.h"
#include <stdio.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>
#include <esp_log.h>
#include <freertos/semphr.h>
#include <esp_timer.h>

#define DHT 27
#define DHT_NEG 26

#define tag "DHT"

SemaphoreHandle_t pos_intr_handler;
SemaphoreHandle_t neg_intr_handler;

int data[40];
int pos_count=0;
int neg_count=0;

int pos_time[44];
int neg_time[44];

void pos_intr(){
  // printf("\n data collecting \n");
    xSemaphoreGive(pos_intr_handler);
}

void neg_intr(){
  // printf("\n data collecting \n");
    xSemaphoreGive(neg_intr_handler);
}

void pos_signal(void* arg){
  while(1){
    xSemaphoreTake(pos_intr_handler, portMAX_DELAY);
    pos_time[pos_count]=  esp_timer_get_time();
    pos_count++;

  }
}

void neg_signal(){
  while(1){
    xSemaphoreTake(neg_intr_handler, portMAX_DELAY);
    neg_time[neg_count]=  esp_timer_get_time();
    neg_count++;
  }
}

void startSignal(){
  gpio_isr_handler_remove(DHT);
  gpio_pulldown_en(DHT);
  gpio_set_direction(DHT, GPIO_MODE_OUTPUT);
  gpio_set_direction(DHT_NEG, GPIO_MODE_OUTPUT);
  gpio_set_level(DHT, 0);
  esp_rom_delay_us(2000);
  gpio_set_level(DHT, 1);
  esp_rom_delay_us(25);
  gpio_set_direction(DHT, GPIO_MODE_INPUT);
  gpio_set_direction(DHT_NEG, GPIO_MODE_INPUT);
  gpio_isr_handler_add(DHT, pos_intr, NULL);
  gpio_isr_handler_add(DHT_NEG, neg_intr, NULL);

  for(int i=0 ; i<41 ; i++){
    printf("%d: %d\n",i ,neg_time[i+1]-pos_time[i-1]);
  }
    pos_count=0;
    neg_count=0;
    // esp_rom_delay_us(140);
    // xSemaphoreGive(pos_intr_handler);
    //
}

void intr_init(){
  gpio_install_isr_service(0);

  gpio_config_t pos_pin={
    .intr_type = GPIO_INTR_POSEDGE,
    .pull_down_en = GPIO_PULLDOWN_ENABLE,
    .pin_bit_mask = (1ULL << DHT),
  };
  gpio_config(&pos_pin);
  gpio_isr_handler_add(DHT, pos_intr, NULL);

  gpio_config_t neg_pin={
    .intr_type = GPIO_INTR_NEGEDGE,
    .pull_down_en = GPIO_PULLDOWN_ENABLE,
    .pin_bit_mask = (1ULL << DHT_NEG),
  };
  gpio_config(&neg_pin);
  gpio_isr_handler_add(DHT_NEG, neg_intr, NULL);
}

void app_main(void)
{
  pos_intr_handler = xSemaphoreCreateBinary();
  neg_intr_handler = xSemaphoreCreateBinary();
  
  memset(pos_time, '0', sizeof(pos_time));
  memset(neg_time, '0', sizeof(neg_time));


  intr_init();

  esp_timer_create_args_t timer_dht = {
     .callback = startSignal,
     .name = "DHT timer",
  };
  esp_timer_handle_t timer_dht_handle;
  esp_timer_create(&timer_dht,&timer_dht_handle);
  esp_timer_start_periodic(timer_dht_handle, 2000000);

  xTaskCreate(&pos_signal, "processing the Signal", 2048, NULL, 2, NULL);
  xTaskCreate(&neg_signal, "processing the Signal", 2048, NULL, 2, NULL);

}
