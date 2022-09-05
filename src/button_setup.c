
#include "esp_err.h"
#include "esp_log.h"
#include "iot_button.h"
#include "settings.h"
#include "button_cb.h"
const static char *TAG = "button_setup";
void pin_setup()
{
    ESP_LOGI(TAG, "Setting up pins");
    gpio_reset_pin(BUTTON_BUZZER);
    gpio_set_direction(BUTTON_BUZZER, GPIO_MODE_OUTPUT);

    gpio_set_level(BUTTON_BUZZER, 1);
    vTaskDelay(250 / portTICK_PERIOD_MS);
    gpio_set_level(BUTTON_BUZZER, 0);
    vTaskDelay(250 / portTICK_PERIOD_MS);
    gpio_set_level(BUTTON_BUZZER, 1);
    vTaskDelay(250 / portTICK_PERIOD_MS);
    gpio_set_level(BUTTON_BUZZER, 0);
    ESP_LOGI(TAG, "End of Setting up pins");
}

esp_err_t button_setup()
{

    pin_setup();
    ESP_LOGI(TAG, "Setting up buttons");

    // initializing button and adding callback
    button_handle_t button_one;
    button_one = iot_button_create(BUTTON_ONE, BUTTON_ACTIVE_HIGH);
    ESP_ERROR_CHECK_WITHOUT_ABORT(iot_button_set_evt_cb(button_one, BUTTON_CB_TAP, (button_cb)button_one_cb_test, BUTTON_ONE));
    // button two
    // initializing button and adding callback
    button_handle_t button_two;
    button_two = iot_button_create(BUTTON_TWO, BUTTON_ACTIVE_HIGH);
    ESP_ERROR_CHECK_WITHOUT_ABORT(iot_button_set_evt_cb(button_two, BUTTON_CB_TAP, (button_cb)button_two_cb_test, BUTTON_TWO));
    ESP_LOGI(TAG, "end of Setting up buttons");

    return ESP_OK;
}