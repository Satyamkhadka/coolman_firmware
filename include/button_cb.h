
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/gpio.h"

QueueHandle_t xQueue;

esp_err_t button_cb(gpio_num_t pin);