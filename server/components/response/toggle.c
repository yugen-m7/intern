#include "hal/gpio_types.h"
#include <driver/gpio.h>

#define LED_1 2

void led_setup(){
    gpio_reset_pin(LED_1);
    gpio_set_direction(LED_1, GPIO_MODE_OUTPUT);
}

void led_toggle(bool level_1){
    gpio_set_level(LED_1, level_1);
}
