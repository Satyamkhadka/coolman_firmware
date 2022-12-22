#include "indicator_pattern.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
void indicate(gpio_num_t pin[], int number_of_pins, int on_time, int off_time, int times)
{

    // ESP_LOGI(__func__, "indicate %d %d %d", on_time, off_time, times);
    if (times < 1)
    {

        return;
    }
    else
    {

        for (int i = 0; i < times; i++)
        {

            // ESP_LOGI(__func__, "total size  %d  %d %d", on_time, off_time, times);

            for (int j = 0; j < number_of_pins; j++)
            {
                gpio_set_level(pin[j], 1);
                // ESP_LOGI(__func__, "%d", pin[j]);
            }

            vTaskDelay(on_time / portTICK_PERIOD_MS);

            for (int j = 0; j < number_of_pins; j++)
            {
                gpio_set_level(pin[j], 0);
            }
        }
        vTaskDelay(off_time / portTICK_PERIOD_MS);
    }
    // ESP_LOGI(__func__, "exiting");
}
