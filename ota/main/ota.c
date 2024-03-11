#include <driver/gpio.h>
#include <esp_event.h>
#include <esp_http_client.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <protocol_examples_common.h>
#include <stdio.h>

#define intr_button 0

SemaphoreHandle_t sema_handler;

const int software_version = 1;

typedef struct {
  char *buffer;
  int buffer_index;
} chunk_payload_t;

static char *buffer;

// for https certification
extern const uint8_t pem[] asm("_binary_randomuser_pem_start");

// for getting the data from HTTPS
esp_err_t client_event(esp_http_client_event_t *evt) {
  chunk_payload_t *chunk = evt->user_data;
  switch (evt->event_id) {
  case HTTP_EVENT_ON_DATA:
    ESP_LOGI("DATA_SIZE", "%d", evt->data_len);

    // for receiving data in chuncks
    chunk->buffer =
        realloc(chunk->buffer, chunk->buffer_index + evt->data_len + 1);
    memcpy(&chunk->buffer[chunk->buffer_index], (uint8_t *)evt->data,
           evt->data_len);
    chunk->buffer_index += evt->data_len;
    chunk->buffer[chunk->buffer_index] = 0;

    break;
  default:
    break;
  }
  buffer = chunk->buffer;
  return ESP_OK;
}

void IRAM_ATTR intr_button_pushed() {
  xSemaphoreGiveFromISR(sema_handler, pdFALSE);
}

void intr_func() {
  while (1) {
    xSemaphoreTake(sema_handler, portMAX_DELAY);
    printf("you pushed the intr button\n");
  }
}

void wifi_connect() {
  nvs_flash_init();
  esp_netif_init();
  esp_event_loop_create_default();
  example_connect();
}

void intr_setup() {
  gpio_config_t intr_button_config = {
      .intr_type = GPIO_INTR_POSEDGE,
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_ENABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .pin_bit_mask = (1ULL << intr_button),
  };
  gpio_config(&intr_button_config);
  gpio_install_isr_service(0);
  gpio_isr_handler_add(intr_button, intr_button_pushed, NULL);
}

void client_setup() {
  chunk_payload_t chunk = {0};

  esp_http_client_config_t client_config = {.url = "https://randomuser.me/api/",
                                            .method = HTTP_METHOD_GET,
                                            .event_handler = client_event,
                                            .user_data = &chunk,
                                            .cert_pem = (char *)pem};
  esp_http_client_handle_t client_handle = esp_http_client_init(&client_config);
  esp_http_client_set_header(client_handle, "Content_type", "application/json");

  esp_http_client_perform(client_handle);
  printf("%s\n", buffer);

  esp_http_client_cleanup(client_handle);
}

void app_main(void) {
  printf("\n");
  ESP_LOGI("Software Version", "%d\n", software_version);
  wifi_connect();
  sema_handler = xSemaphoreCreateBinary();
  // configuration for isr push button
  intr_setup();
  client_setup();

  xTaskCreate(intr_func, "function to be executed", 2048, NULL, 2, NULL);
}
