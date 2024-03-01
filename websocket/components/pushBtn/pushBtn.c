#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#define BTN 0

SemaphoreHandle_t sHandle;

static void IRAM_ATTR btn_pushed(void *arg) {
  xSemaphoreGiveFromISR(sHandle, NULL);
}

static void task(void *arg) {
  while (1) {
    xSemaphoreTake(sHandle, portMAX_DELAY);
    printf("your pressed the ISR button\n");
  }
}

void isr_init() {
  sHandle = xSemaphoreCreateBinary();
  xTaskCreate(task, "just a task", 2048, NULL, 2, NULL);
  gpio_config_t button = {
      .intr_type = GPIO_INTR_POSEDGE,
      .mode = GPIO_MODE_INPUT,
      .pull_down_en = 1,
      .pin_bit_mask = (1ULL << BTN),
  };

  gpio_config(&button);

  gpio_install_isr_service(0);
  gpio_isr_handler_add(BTN, btn_pushed, NULL);
}
