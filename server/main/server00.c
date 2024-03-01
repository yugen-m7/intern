#include <stdio.h>
#include <connect.h>
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_http_server.h"

static const char *TAG = "SERVER";


static esp_err_t server_url(httpd_req_t *req)
{
    printf("hello world\n");
    ESP_LOGI(TAG,"URL: %s",req->uri);
    httpd_resp_sendstr(req,"hello world");
}

static void init_server()
{
  httpd_handle_t server = NULL;
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();

  ESP_ERROR_CHECK(httpd_start(&server, &config));

  httpd_uri_t default_url = {
    .uri ="/",
    .method = HTTP_GET,
    .handler = server_url,
  };
  httpd_register_uri_handler(server,&default_url);

}

void app_main(void)
{
  connect_init();
  ESP_ERROR_CHECK(connect_sta("nepaldigisys", "NDS_0ffice", 10000));

  init_server();
}
