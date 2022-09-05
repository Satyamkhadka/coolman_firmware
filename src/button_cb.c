#include <time.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"

#include "button_cb.h"
#include "https.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "settings.h"
const static char *TAG = "button_cb";

time_t now;
time_t prev;
char out[12];
void button_one_cb(gpio_num_t pin)
{
    ESP_LOGI(TAG, "button pressed number:%d", pin);
    time(&now);
    ESP_LOGI(TAG, "current timestamp is: %d", (int)now);

    if (uxQueueSpacesAvailable(xQueue) == 0)
    {
        xQueueReceive(xQueue, out, (TickType_t)0);
        xQueueReceive(xQueue, out, (TickType_t)0);
        xQueueReceive(xQueue, out, (TickType_t)0);
    }
    out[0] = '\0';
    sprintf(out, "%d=%d", (int)now, (int)pin);
    if (xQueueSend(xQueue, out, (TickType_t)0) == pdTRUE)
    {
        return ESP_OK;
    }
    else
    {
        return ESP_FAIL;
    }
}
void button_one_cb_test(gpio_num_t pin)
{
    ESP_LOGI(TAG, "button pressed number:%d", pin);
    time(&now);
    ESP_LOGI(TAG, "current timestamp is: %d", (int)now);
    gpio_set_level(BUTTON_BUZZER, 1);
    vTaskDelay(250 / portTICK_PERIOD_MS);
    gpio_set_level(BUTTON_BUZZER, 0);
}
void button_two_cb_test(gpio_num_t pin)
{
    ESP_LOGI(TAG, "button pressed number:%d", pin);
    time(&now);
    ESP_LOGI(TAG, "current timestamp is: %d", (int)now);
    gpio_set_level(BUTTON_BUZZER, 1);
    vTaskDelay(250 / portTICK_PERIOD_MS);
    gpio_set_level(BUTTON_BUZZER, 0);
}