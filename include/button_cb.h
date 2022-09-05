
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/gpio.h"

QueueHandle_t xQueue;

void button_one_cb(gpio_num_t pin);
void button_one_cb_test(gpio_num_t pin);
void button_two_cb_test(gpio_num_t pin);
void button_three_cb_test(gpio_num_t pin);
void button_four_cb_test(gpio_num_t pin);
void button_five_cb_test(gpio_num_t pin);
