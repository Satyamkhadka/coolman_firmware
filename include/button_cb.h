
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/gpio.h"

QueueHandle_t xQueue;

void push_button_callback(int button_number);
