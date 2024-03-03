#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <cJSON.h>
#include <driver/uart.h>
#include <driver/gpio.h>
#include <esp_timer.h>

#define TX_GPIO_PIN 4
#define RX_GPIO_PIN 5

#define TX_PIN 12
#define RX_PIN 13

#define RX_SIZE 100

void uart_gpio_control(void* para){
    char rx_buffer[RX_SIZE];
    bool pin_level=false;
    while(1){
        /* READING FROM UART */
        memset(rx_buffer, 0, sizeof(rx_buffer));
        uint16_t data_size = uart_read_bytes(UART_NUM_1,
                                             (uint8_t*)rx_buffer,
                                             RX_SIZE,
                                             pdMS_TO_TICKS(500));

        /* PARSING RECEIVED DATA */
        cJSON *test=cJSON_Parse((char*)rx_buffer);
        if(test != NULL){
            cJSON *command=cJSON_GetObjectItemCaseSensitive(test, "command");
            cJSON *gpio=cJSON_GetObjectItem(test, "gpio");
            printf("gpio: %d\n",gpio->valueint);
            printf("command: %s\n",command->valuestring);

            if(strcmp(command->valuestring, "on")==0){
                pin_level=true;
            }else if(strcmp(command->valuestring, "off")==0){
                pin_level=false;
            }
            gpio_set_direction(gpio->valueint, GPIO_MODE_OUTPUT);
            gpio_set_level(gpio->valueint, pin_level);
        }else if(test == NULL  && data_size!=0){
            printf("Invalid input\n");
        }
    }
}

void uart_input_control(void* para){
    char rx_buffer[10];
    char tx_buffer[10]="A";
    memset(rx_buffer, 0, sizeof(*rx_buffer));

    uart_write_bytes(UART_NUM_2, tx_buffer, sizeof(tx_buffer));
    uint16_t rx_size = uart_read_bytes(UART_NUM_2, rx_buffer, sizeof(rx_buffer), pdMS_TO_TICKS(500));
    if(rx_size>0) printf("received: %s\n", rx_buffer);
}

/* interrupt timer setup */
void isr_timer(uint32_t ms){
    esp_timer_create_args_t isr_timer = {
        .arg=NULL,
        .callback=uart_input_control};
    esp_timer_handle_t isr_handler;
    esp_timer_create(&isr_timer, &isr_handler);
    esp_timer_start_periodic(isr_handler, 1000*ms);
}

void app_main(void)
{
    /* UART SETUP */
    uart_config_t uart_test={
        .baud_rate=115200,
        .data_bits=UART_DATA_8_BITS,
        .stop_bits=UART_STOP_BITS_1,
        .flow_ctrl= UART_HW_FLOWCTRL_DISABLE,
        .parity=UART_PARITY_DISABLE,
    };

    /* UART_NUM_1 setup */
    uart_param_config(UART_NUM_1, &uart_test);
    uart_driver_install(UART_NUM_1, 2048, 2048, 0, NULL, 0);
    uart_set_pin(UART_NUM_1, TX_GPIO_PIN, RX_GPIO_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    /* UART_NUM_2 setup */
    uart_param_config(UART_NUM_2, &uart_test);
    uart_driver_install(UART_NUM_2, 2048, 2048, 0, NULL, 0);
    uart_set_pin(UART_NUM_2, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    isr_timer(1000);

    xTaskCreate(uart_gpio_control, "uart_tr", 2048, NULL, 2, NULL);
}
