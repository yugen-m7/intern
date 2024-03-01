#include "freertos/projdefs.h"
#include <driver/gpio.h>
#include <driver/uart.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <stdio.h>
#include <string.h>

#define TX_PIN 13
#define RX_PIN 12

#define gpio_out 15

#define UART_NUMX UART_NUM_2

QueueHandle_t uart_handler;

void uart_event_task() {
    uart_event_t uart_event;
    char* rx_buffer = malloc(20);
    while (1) {
      if (xQueueReceive(uart_handler,&uart_event, portMAX_DELAY)) {
        switch (uart_event.type) {
        case UART_DATA:
          break;
        case UART_PATTERN_DET:
          ESP_LOGI("TAG", "UART_PATTERN_DET");
          int pos = uart_pattern_pop_pos(UART_NUMX);
          if (pos < 0 || pos > 20) {
            uart_flush_input(UART_NUMX);
          } else {
            uart_read_bytes(UART_NUMX, rx_buffer, pos, portMAX_DELAY);
            printf("pop_pos: %d\n", pos);
            printf("distance: %c%c%c%c", rx_buffer[1], rx_buffer[2],
                   rx_buffer[3], rx_buffer[4]);
          }
          break;
        default:
          break;
        }
      }
    }
}

void app_main(void) {
    uart_config_t uart_setup = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .stop_bits = UART_STOP_BITS_1,
        .parity = UART_PARITY_DISABLE,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    uart_param_config(UART_NUMX, &uart_setup);
    uart_driver_install(UART_NUMX, 2048, 2048, 10, &uart_handler, 0);
    uart_set_pin(UART_NUMX, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE,
                 UART_PIN_NO_CHANGE);

    /* for turning the sensor ON */
    gpio_reset_pin(gpio_out);
    gpio_set_direction(gpio_out, GPIO_MODE_OUTPUT);
    gpio_set_level(gpio_out, 1);

    /* detecting R */
    ESP_ERROR_CHECK(
        uart_enable_pattern_det_baud_intr(UART_NUMX, 'R', 1, 10000, 4, 0));
    uart_pattern_queue_reset(UART_NUMX, 100);

    xTaskCreate(uart_event_task, "uart_event_task", 2048, NULL, 3, NULL);
}
