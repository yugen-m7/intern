#include "esp_rom_sys.h"
#include <stdio.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <string.h>
#include <esp_log.h>
#include <freertos/semphr.h>

#define DHT 4
#define tag "DHT"

SemaphoreHandle_t uInput_handler;

int data[40];

void startSignal();
int getSignal(int utimeout);
void processSignal();

void startSignal(){
  gpio_reset_pin(DHT);
  gpio_pulldown_en(DHT);
  while(1){
    // start signal
    gpio_set_direction(DHT, GPIO_MODE_OUTPUT);
    gpio_set_level(DHT, 0);
    esp_rom_delay_us(2000);
    gpio_set_level(DHT, 1);
    esp_rom_delay_us(25);
    gpio_set_direction(DHT, GPIO_MODE_INPUT);
    esp_rom_delay_us(140);
    xSemaphoreGive(uInput_handler);

    vTaskDelay(pdMS_TO_TICKS(3000));
  }
}

int getSignal(int utimeout){
  int utime=0;
    while(gpio_get_level(DHT)==1){
      ++utime;
      esp_rom_delay_us(1);
    }
  printf("%d\n",utime);
  if(utimeout>utime){

    return 0;
  }else{
    return 1;
  }
}

void processSignal(){
  // data_value data;
  while(1){
    memset(data, '0', sizeof(data));
    uint8_t bits=0;
    xSemaphoreTake(uInput_handler, portMAX_DELAY);
    while(bits<40){
      while(gpio_get_level(DHT)!=1){
        esp_rom_delay_us(1);
      }
      data[bits]=getSignal(60);
      bits++;
    }
    for(int i=0 ; i<40 ; i++){
       printf("%d ", data[i]);
    }
    printf("\n");
  }
    esp_rom_delay_us(25);
}

void app_main(void)
{
  uInput_handler = xSemaphoreCreateBinary();

  xTaskCreate(&startSignal, "Start the Signal", 2048, NULL, 2, NULL);
  xTaskCreate(&processSignal, "processing the Signal", 8088, NULL, 2, NULL);
}
