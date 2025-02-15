#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <nvs_flash.h>
#include <esp_wifi.h>

#include "test_input_generator.h"
#include "provisioning.h"
#include "device_setup.h"
#include "https.h"

void app_main(void)
{
    /* Initialize NVS partition */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        /* NVS partition was truncated
         * and needs to be erased */
        ESP_ERROR_CHECK(nvs_flash_erase());

        /* Retry nvs_flash_init */
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    /* Initialize TCP/IP */
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    pin_setup();
    // provision the device here. provide with the wifi
    provisioning();
    button_setup();
    // this sends button presses stored in the queue to the server
    xTaskCreate(https_send_task, "https_send_task", 8192 * 4, NULL, 1, NULL);
}