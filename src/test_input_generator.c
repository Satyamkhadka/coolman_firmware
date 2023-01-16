

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "button_cb.h"
#include "freertos/queue.h"

void btn_press_task()
{
    xQueue = xQueueCreate(512, 12 * sizeof(char));

    while (1)
    {
        push_button_callback(NULL, (void *)1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
