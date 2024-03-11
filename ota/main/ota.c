#include <driver/gpio.h>
#include <esp_event.h>
#include <esp_http_client.h>
#include <esp_https_ota.h>
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
extern const uint8_t pem[] asm("_binary_drive_pem_start");

void IRAM_ATTR intr_button_pushed() {
  xSemaphoreGiveFromISR(sema_handler, pdFALSE);
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

void wifi_setup() {
  nvs_flash_init();
  esp_netif_init();
  esp_event_loop_create_default();
  example_connect();
}
esp_err_t client_event(esp_http_client_event_t *evt) { return ESP_OK; }

void ota_setup() {
  while (1) {
    xSemaphoreTake(sema_handler, portMAX_DELAY);
    esp_http_client_config_t client_config = {
        .url = "https://drive.usercontent.google.com/"
               "download?id=1eAgyp_c-KBP9P2bxWUN2RDhiFBNb9qn0&export=download&"
               "authuser=0&confirm=t&uuid=e005ca17-41e0-446d-b570-7389a2a7c384&"
               "at=APZUnTWM8gsfEjpUQAlRFv_F0fZ5:1710133243072",
        .event_handler = client_event,
        .cert_pem = (char *)pem};
    esp_http_client_handle_t client_handle =
        esp_http_client_init(&client_config);
    esp_http_client_set_header(client_handle, "Content_type",
                               "application/json");

    esp_https_ota_config_t ota_config = {
        .http_config = &client_config,
    };
    if (esp_https_ota(&ota_config) == ESP_OK) {
      ESP_LOGI("UPDATE STATUS", "Update of Succesful");
    }
    ESP_LOGE("UPDATE STATUS", "FAILED");

    // esp_http_client_perform(client_handle);
    // esp_http_client_cleanup(client_handle);
  }
}

void app_main(void) {
  printf("\n");
  ESP_LOGI("Software Version", "%d\n", software_version);
  sema_handler = xSemaphoreCreateBinary();
  // configuration for isr push button

  intr_setup();
  wifi_setup();
  xTaskCreate(ota_setup, "function to be executed", 2048, NULL, 2, NULL);
}
