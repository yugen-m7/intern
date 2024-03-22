#include <stdio.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <string.h>
#include <esp_log.h>

#define DHT 27
#define tag "DHT"

typedef struct {
  uint8_t rhi[8];
  uint8_t rhd[8];
  uint8_t ti[8];
  uint8_t td[8];
  uint8_t cs[8];
}data[];

void reading( uint8_t times,bool state){
  for(int i=0 ; i<8 ; i++){
    int j;
    while(!gpio_get_level(DHT)){
      vTaskDelay(pdMS_TO_TICKS(5)); 
    }
    printf("low done\n");
    while(gpio_get_level(DHT)==1){
      printf("high started\n");
      j++;
      if(j>6) rhi[i]=1;
      vTaskDelay(pdMS_TO_TICKS(5)); 
    }
    printf("high done\n");
    vTaskDelay(pdMS_TO_TICKS(10)); 
  }
}

void app_main(void)
{

  // memset(rhi, '0', sizeof(rhi));
  // memset(rhd, '0', sizeof(rhi));
  // memset(ti, '0', sizeof(rhi));
  // memset(td, '0', sizeof(rhi));
  // memset(cs, '0', sizeof(rhi));

  while(1){
    // start signal
    gpio_set_direction(DHT, GPIO_MODE_OUTPUT);
    gpio_set_level(DHT, 0);
    esp_rom_delay_us(3000);
    gpio_set_level(DHT, 1);
    esp_rom_delay_us(25);
    // gpio_set_direction(DHT, 0);
    gpio_set_direction(DHT, GPIO_MODE_INPUT);
    vTaskDelay(pdMS_TO_TICKS(140)); 

    ESP_LOGI(tag, "Starting Reading Data");


    ESP_LOGI(tag, "Starting Printing Data");

    for(int i=0 ; i<8 ; i++){
      printf("%d",rhi[0]);
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
