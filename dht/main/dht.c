#include "hal/gpio_types.h"
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

SemaphoreHandle_t uInput_handler;

int data[40];
int count;

void intr_signal(){
  // printf("\n data collecting \n");
  xSemaphoreGive(uInput_handler);
}

void signal(void* arg){
  while(1){
    xSemaphoreTake(uInput_handler, portMAX_DELAY);
    count++;
    printf("%d\n", count);
  }
}

void startSignal(){
  gpio_isr_handler_remove(DHT);
  gpio_pulldown_en(DHT);
  gpio_set_direction(DHT, GPIO_MODE_OUTPUT);
  gpio_set_level(DHT, 0);
  esp_rom_delay_us(2000);
  gpio_set_level(DHT, 1);
  esp_rom_delay_us(25);
  gpio_set_direction(DHT, GPIO_MODE_INPUT);
  gpio_isr_handler_add(DHT, intr_signal, NULL);
  gpio_isr_handler_add(DHT_NEG, intr_signal, NULL);

  count=0;
  // esp_rom_delay_us(140);
  // xSemaphoreGive(uInput_handler);
  //
}

void intr_init(){
  gpio_install_isr_service(0);

  gpio_config_t signal_pin={
    .intr_type = GPIO_INTR_POSEDGE,
    .pull_down_en = GPIO_PULLDOWN_ENABLE,
    .pin_bit_mask = (1ULL << DHT),
  };
  gpio_config(&signal_pin);
  gpio_isr_handler_add(DHT, intr_signal, NULL);

  gpio_config_t neg_pin={
    .intr_type = GPIO_INTR_NEGEDGE,
    .pull_down_en = GPIO_PULLDOWN_ENABLE,
    .pin_bit_mask = (1ULL << DHT_NEG),
  };
  gpio_config(&neg_pin);
  gpio_isr_handler_add(DHT_NEG, intr_signal, NULL);
}

void app_main(void)
{
  uInput_handler = xSemaphoreCreateBinary();
  
  intr_init();

  esp_timer_create_args_t timer_dht = {
     .callback = startSignal,
     .name = "DHT timer",
  };
  esp_timer_handle_t timer_dht_handle;
  esp_timer_create(&timer_dht,&timer_dht_handle);
  esp_timer_start_periodic(timer_dht_handle, 2000000);

  xTaskCreate(&signal, "processing the Signal", 2048, NULL, 2, NULL);
}
