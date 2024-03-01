#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/uart.h>
#include <driver/gpio.h>
#include <string.h>

#define TX_PIN 13
#define RX_PIN 12

#define gpio_out 15

void app_main(void)
{
    uart_config_t uart_setup = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .stop_bits = UART_STOP_BITS_1,
        .parity = UART_PARITY_DISABLE,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    uart_param_config(UART_NUM_1,  &uart_setup);
    uart_driver_install(UART_NUM_1, 2048, 2048, 0, NULL, 0);
    uart_set_pin(UART_NUM_1, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    gpio_reset_pin(gpio_out);
    gpio_set_direction(gpio_out, GPIO_MODE_OUTPUT);
    gpio_set_level(gpio_out, 1);

    char data[20];

    while(1){
        memset(data, 0, sizeof(data));
        uint16_t data_size=uart_read_bytes(UART_NUM_1, data, sizeof(data), pdMS_TO_TICKS(100));
        /* printf("data_size: %d\n", data_size); */
        if(data[0]=='R'&& data_size==6){
            printf("distance: %c%c%c%c\n",data[1],data[2],data[3],data[4]);
        }
        vTaskDelay(pdMS_TO_TICKS(110));
    }


}
