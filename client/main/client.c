#include "protocol_examples_common.h"
#include <cJSON.h>
#include <esp_event.h>
#include <esp_http_client.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <esp_wifi.h>
#include <nvs_flash.h>
#include <stdio.h>

typedef struct {
  char *buffer;
  int buffer_index;
} chunk_payload_t;

static char *buffer;

// for https certification
extern const uint8_t pem[] asm("_binary_randomuser_pem_start");

void phraser(char *data) {
}

esp_err_t client_event(esp_http_client_event_t *evt) {
  chunk_payload_t *chunk = evt->user_data;
  switch (evt->event_id) {
  case HTTP_EVENT_ON_DATA:
    ESP_LOGI("DATA_SIZE", "%d", evt->data_len);

    /* chunk */
    chunk->buffer =
        realloc(chunk->buffer, chunk->buffer_index + evt->data_len + 1);
    memcpy(&chunk->buffer[chunk->buffer_index], (uint8_t *)evt->data,
           evt->data_len);
    chunk->buffer_index += evt->data_len;
    chunk->buffer[chunk->buffer_index] = 0;

    /* printf("%s\n", chunk->buffer); */
    break;
  default:
    break;
  }
  buffer = chunk->buffer;
  return ESP_OK;
  /* phraser(chunk->buffer); */
}

void app_main(void) {
  chunk_payload_t chunk = {0};

  nvs_flash_init();
esp_netif_init();
  esp_event_loop_create_default();
  example_connect();
  esp_http_client_config_t client_config = {.url = "https://randomuser.me/api/",
                                            .method = HTTP_METHOD_GET,
                                            .event_handler = client_event,
                                            .user_data = &chunk,
                                            .cert_pem = (char *)pem};
  esp_http_client_handle_t client_handle = esp_http_client_init(&client_config);
  esp_http_client_set_header(client_handle, "Content_type", "application/json");

  esp_http_client_perform(client_handle);

  printf("%s\n", buffer);

  cJSON *info = cJSON_Parse(buffer);
  if (info == NULL) {
    printf("null\n\n\n");
  } else {
    cJSON *results = cJSON_GetObjectItem(info, "results");
    cJSON *result = cJSON_GetArrayItem(results, 0);
    cJSON *gender = cJSON_GetObjectItem(result, "gender");
    cJSON *info2 = cJSON_GetObjectItemCaseSensitive(info, "info");
    cJSON *seed = cJSON_GetObjectItemCaseSensitive(info2, "seed");
    printf("\nseed: %s\n", seed->valuestring);
    printf("gender: %s\n", gender->valuestring);
  }
  esp_http_client_cleanup(client_handle);

  example_disconnect();
}
