

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "button_cb.h"
#include "freertos/queue.h"

void btn_press_task()
{
    xQueue = xQueueCreate(512, 12 * sizeof(char));

    while (1)
    {

        button_cb(5);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        button_cb(6);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        button_cb(7);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        button_cb(8);
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}
