#include <stdio.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <string.h>
#include <esp_log.h>
#include <freertos/semphr.h>
#include <esp_timer.h>
#include <dht.h>

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


TaskHandle_t signal_handler;

// function prototype
void startSignal();
int process_signal();


float get_humidity(){ return humidity; }

float get_temperature(){ return temperature; }


static IRAM_ATTR void pos_intr(){
  pos_time[pos_count]=  esp_timer_get_time();
  pos_count++;
}

static IRAM_ATTR void neg_intr(){
  neg_time[neg_count]=  esp_timer_get_time();
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
      // printf("Humidity:%4.2f\nTemperature:%4.2f", get_humidity(), get_temperature()); printf("\n\n");
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

  humidity = data[0];
  humidity *= 0x100;					
  humidity += data[1];
  humidity /= 10;						

  temperature = data[2];	
  temperature *= 0x100;				// >> 8
  temperature += data[3];
  temperature /= 10;

  if(checksum!=data[4]) return DHT_CHECKSUM_ERROR;

  return ESP_OK;
}


void dht_output(){
  while(1){
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
}


void dht_init()
{
  intr_init();

  xTaskCreatePinnedToCore(startSignal, "Start Signal", 2048, NULL, 4, NULL, 1);
  xTaskCreate(dht_output, "output value", 2048, NULL, 2, &signal_handler);
}
