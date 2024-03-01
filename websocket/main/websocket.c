#include <esp_http_server.h>
#include <esp_log.h>
#include <mdns.h>
#include <cJSON.h>

#include <connect.h>
#include <toggle.h>
#include <pushBtn.h>

static const char* TAG = "HTTPD SERVER";

extern const char html[] asm("_binary_test_html_start");

esp_err_t uri_test(httpd_req_t* req){
  ESP_LOGI(TAG , "uri_handler is strating\n");
  httpd_resp_sendstr(req, html);
  printf("hello\n");
  return ESP_OK;
}

esp_err_t uri_resp(httpd_req_t* req){
   char buffer[100];
   memset(buffer, 0, sizeof(buffer));
   httpd_req_recv(req, buffer, req->content_len);
   cJSON *carrier = cJSON_Parse(buffer);
   if (carrier != NULL) {
     cJSON *level = cJSON_GetObjectItem(carrier, "level");
     cJSON *gpio = cJSON_GetObjectItem(carrier, "gpio");
     printf("\nGPIO -> %d\nLEVEL -> %s\n",gpio->valueint,
            (int)level->valueint==1? "true": "false");
   }else {
       printf("invalid request\n");
   }
    cJSON_Delete(carrier);
   return ESP_OK;
}

static int client_id;

static esp_err_t uri_ws(httpd_req_t *req) {
   client_id = httpd_req_to_sockfd(req);
   if (req->method == HTTP_GET)
       return ESP_OK;

   httpd_ws_frame_t ws_frame = {
       /* memset(&ws_frame, 0, sizeof(httpd_ws_frame_t)); */
       .type = HTTPD_WS_TYPE_TEXT,
       .payload = malloc(1024),
   };
   httpd_ws_recv_frame(req, &ws_frame, 1024);
   printf("ws payload: %.*s\n", ws_frame.len, ws_frame.payload);
   free(ws_frame.payload);

   char *response = "connected";
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

   httpd_uri_t httpd_uri = {
       .uri = "/",
       .method = HTTP_GET,
       .handler = uri_test,
   };
   httpd_uri_t toggle_uri = {
       .uri = "/toggle",
       .method = HTTP_POST,
       .handler = uri_resp,
   };
   httpd_uri_t ws_uri = {
       .uri = "/ws",
       .method = HTTP_GET,
       .handler = uri_ws,
   };

   httpd_start(&httpd_handler, &httpd_config);
   httpd_register_uri_handler(httpd_handler, &httpd_uri);
   httpd_register_uri_handler(httpd_handler, &toggle_uri);
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
  connect_sta("nepaldigisys", "NDS_0ffice", 10000);

  //
  mdns_service();
  isr_init();
  // starting the server
  server();

}
