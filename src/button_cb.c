#include <time.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"

#include "button_cb.h"
#include "https.h"
#include "freertos/queue.h"
const static char *TAG = "button_cb";

time_t now;
time_t prev;
char out[12];
esp_err_t button_cb(gpio_num_t pin)
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
