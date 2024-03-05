#include <esp_http_server.h>
#include <esp_log.h>
#include <mdns.h>
#include <cJSON.h>

#include <connect.h>
#include <toggle.h>
#include <pushBtn.h>


static int client_id;

static esp_err_t uri_ws(httpd_req_t *req) {
   client_id = httpd_req_to_sockfd(req);
   if (req->method == HTTP_GET)
       return ESP_OK;

   char *response = 0;

   httpd_ws_frame_t ws_recv= {
       .type = HTTPD_WS_TYPE_TEXT,
       .payload = malloc(1024),
   };

   httpd_ws_recv_frame(req, &ws_recv, 1024);
   printf("payload: %.*s\n",ws_recv.len, ws_recv.payload);

   cJSON *carrier = cJSON_Parse((char*)ws_recv.payload);
   if (carrier != NULL) {
     cJSON *level = cJSON_GetObjectItem(carrier, "level");
     cJSON *gpio = cJSON_GetObjectItem(carrier, "gpio");
     printf("\nGPIO -> %d\nLEVEL -> %s\n",gpio->valueint,
            (int)level->valueint==1? "true": "false");

      response  = "valid request ['_']\n";
   }else {
      response  = "invalid request [-_-]\n";
   }

   free(ws_recv.payload);
   httpd_ws_frame_t ws_response = {.final = true,
                                   .fragmented = false,
                                   .type = HTTPD_WS_TYPE_TEXT,
                                   .payload = (uint8_t *)response,
                                   .len = strlen(response)};
   return httpd_ws_send_frame(req, &ws_response);
}

void server() {
   httpd_handle_t httpd_handler = NULL;
   httpd_config_t httpd_config = HTTPD_DEFAULT_CONFIG();
   httpd_start(&httpd_handler, &httpd_config);

   httpd_uri_t ws_uri = {
       .uri = "/ws",
       .method = HTTP_GET,
       .handler = uri_ws,
       .is_websocket = true,
   };
   httpd_register_uri_handler(httpd_handler, &ws_uri);
}

void mdns_service() {
  // initialize mnds
  esp_err_t esp = mdns_init();
  if (esp) {
    printf("something went wrong with mdns");
    return;
  }

  // setting the names
  mdns_hostname_set("ion-esp32");
  mdns_instance_name_set("learn to use the server");
}

void app_main() {
  // initializing and connecting to the given AP
  connect_init();
  connect_sta("podamibe", "Chobhar570))", 10000);

  //
  mdns_service();
  isr_init();
  // starting the server
  server();

}
