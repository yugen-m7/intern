#include "esp_attr.h"
#include "freertos/idf_additions.h"
#include <math.h>
#include <stdio.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <string.h>
#include <esp_log.h>
#include <esp_err.h>
#include <freertos/semphr.h>
#include <esp_timer.h>
#include <dht.h>
#include <driver/rmt_rx.h>

#define DHT_TIMEOUT_ERROR -2
#define DHT_CHECKSUM_ERROR -1
#define DHT_OK 0

#define DHT 27
#define DHT_NEG 26

#define tag "DHT"

int pos_count=0;
int neg_count=0;

int pos_time[44];
int neg_time[44];

bool dht_data[40];
uint8_t data[5];

float humidity;
float temperature;

// SemaphoreHandle_t signal_handler;
TaskHandle_t signal_handler;

// function prototype
void startSignal();
int process_signal();

static IRAM_ATTR void pos_intr(){
  pos_time[pos_count]=  esp_timer_get_time();
  pos_count++;
}

static IRAM_ATTR void neg_intr(){
  neg_time[neg_ount]=  esp_timer_get_time();
  neg_count++;
}

void error_handler(int response){
  switch(response){
    case DHT_TIMEOUT_ERROR:
      ESP_LOGE(tag, "DHT_TIMEOUT_ERROR");
      break;
    case DHT_CHECKSUM_ERROR:
      ESP_LOGE(tag, "DHT_CHECKOUT_ERROR");
      break;
    case DHT_OK:
      printf("Humidity:%4.2f\nTemperature:%4.2f", humidity, temperature); printf("\n\n");
      break;
    default:
      ESP_LOGE(tag, "DHT_UNKNOWN_ERROR");
  }
}

void startSignal(){
  while(1){
    gpio_isr_handler_remove(DHT);
    gpio_isr_handler_remove(DHT_NEG);
    gpio_pulldown_en(DHT);
    gpio_set_direction(DHT, GPIO_MODE_OUTPUT);
    gpio_set_direction(DHT_NEG, GPIO_MODE_OUTPUT);
    gpio_set_level(DHT, 0);
    esp_rom_delay_us(2000);
    gpio_set_level(DHT, 1);
    esp_rom_delay_us(25);
    gpio_set_level(DHT, 0);
    gpio_set_direction(DHT, GPIO_MODE_INPUT);
    gpio_set_direction(DHT_NEG, GPIO_MODE_INPUT);
    gpio_isr_handler_add(DHT, pos_intr, NULL);
    gpio_isr_handler_add(DHT_NEG, neg_intr, NULL);
    vTaskDelay(pdMS_TO_TICKS(50));
    // xSemaphoreGive(signal_handler);
    xTaskNotifyGive(signal_handler);
    vTaskDelay(pdMS_TO_TICKS(3000));
  }
}

int process_signal(){
  // vTaskDelay(pdMS_TO_TICKS(10));
  for(int i=0 ; i<41 ; i++){
    int utime = neg_time[i]-pos_time[i] ;
    if(utime>0){
      dht_data[i] = utime>60 ? 1 : 0; 
    }else{
      return DHT_TIMEOUT_ERROR;
    }
  }

  memset(data, 0, sizeof(data));

  for(int j=0 ; j<8 ; j++ ){
    int k=7-(j%8);
    if(dht_data[j+1]){data[0]|=(1<<k);}
  }
  for(int j=8 ; j<16 ; j++ ){
    int k=7-(j%8);
    if(dht_data[j+1])data[1]|=(1<<k);
  }
  for(int j=16 ; j<24 ; j++ ){
    int k=7-(j%8);
    if(dht_data[j+1])data[2]|=(1<<k);
  }
  for(int j=24 ; j<32 ; j++ ){
    int k=7-(j%8);
    if(dht_data[j+1])data[3]|=(1<<k);
  }
  for(int j=32 ; j<40 ; j++ ){
    int k=7-(j%8);
    if(dht_data[j+1])data[4]|=(1<<k);
  }

  uint8_t checksum = (data[0]+data[1]+data[2]+data[3])& 0xFF;

  if(checksum!=data[4]) return DHT_CHECKSUM_ERROR;

  humidity = data[0];
  humidity *= 0x100;					
  humidity += data[1];
  humidity /= 10;						

  temperature = data[2];	
  temperature *= 0x100;				// >> 8
  temperature += data[3];
  temperature /= 10;
  // ESP_LOGI(tag,"Humidity:%4.2f\nTemperature:%4.2f", humidity, temperature);
  return ESP_OK;
}

void dht_output(){
  while(1){
    // xSemaphoreTake(signal_handler, portMAX_DELAY);
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    error_handler(process_signal());

    pos_count=0;
    neg_count=0;
  }
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

rmt_channel_handle_t rx_chan = NULL;
rmt_rx_channel_config_t rx_chan_config = {
    .clk_src = RMT_CLK_SRC_DEFAULT,   // select source clock
    .resolution_hz = 1 * 1000 * 1000, // 1 MHz tick resolution, i.e., 1 tick = 1 µs
    .mem_block_symbols = 64,          // memory block size, 64 * 4 = 256 Bytes
    .gpio_num = 27,                    // GPIO number
    .flags.invert_in = false,         // do not invert input signal
    .flags.with_dma = false,          // do not need DMA backend
};
ESP_ERROR_CHECK(rmt_new_rx_channel(&rx_chan_config, &rx_chan));
}


void dht_init()
{
  memset(pos_time, 0, sizeof(pos_time));
  memset(neg_time, 0, sizeof(neg_time));
  memset(dht_data, 0, sizeof(dht_data));
  memset(data, 0, sizeof(data));
  // signal_handler = xSemaphoreCreateBinary();


  intr_init();

  // timer_init();

  xTaskCreate(startSignal, "Start Signal", 2048, NULL, 2, NULL);
  // xTaskCreate(dht_output, "Output value", 2048, NULL, 20, &signal_handler);
  xTaskCreatePinnedToCore(dht_output, "output value", 2048, NULL, 8, &signal_handler, 1);
}
