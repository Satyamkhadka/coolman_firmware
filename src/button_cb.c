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
#include <wifi_provisioning/manager.h>

const static char *TAG = "button_cb";

time_t now;
time_t prev;
char out[14];
void push_button_callback(void *arg, void *button_number)
{
    ESP_LOGI(TAG, "button pressed number:%d", (int)button_number);
    time(&now);
    ESP_LOGI(TAG, "current timestamp is: %d", (int)now);
    gpio_set_level(BUTTON_BUZZER, 1);
    vTaskDelay(250 / portTICK_PERIOD_MS);
    gpio_set_level(BUTTON_BUZZER, 0);
    // if space not available then emptying 3 space
    if (uxQueueSpacesAvailable(xQueue) == 0)
    {
        xQueueReceive(xQueue, out, (TickType_t)0);
        xQueueReceive(xQueue, out, (TickType_t)0);
        xQueueReceive(xQueue, out, (TickType_t)0);
    }
    // preparing the output
    out[0] = '\0';
    sprintf(out, "\"%d\":\"%d\"", (int)now, (int)button_number);
    // sending in queue

    xQueueSend(xQueue, out, (TickType_t)0);
}
void reset_provision(void *arg, void *button_number)
{
    wifi_prov_mgr_reset_provisioning();
    esp_restart();
}