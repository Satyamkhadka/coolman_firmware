
#include "esp_err.h"
#include "esp_log.h"
#include "iot_button.h"
#include "settings.h"
#include "button_cb.h"

#include <time.h>
#include "esp_sntp.h"
#include "driver/gpio.h"
#include "indicator_pattern.h"

#include <wifi_provisioning/manager.h>

const static char *TAG = "button_setup";

void reset_device()
{
    // gpio_num_t pins[2] = {BUTTON_BUZZER, BUTTON_LED};
    // indicate(pins, 2, 100, 100, 2);
    ESP_LOGI(TAG, "resetting");
    ESP_LOGI(TAG, "resetting");
    ESP_LOGI(TAG, "resetting");
    ESP_LOGI(TAG, "resetting");
    ESP_LOGI(TAG, "resetting");
    ESP_LOGI(TAG, "resetting");
    ESP_LOGI(TAG, "resetting");
    ESP_LOGI(TAG, "resetting");
    ESP_LOGI(TAG, "resetting");
    ESP_LOGI(TAG, "resetting");
    ESP_LOGI(TAG, "resetting");
    ESP_LOGI(TAG, "resetting");
    ESP_LOGI(TAG, "resetting");
    ESP_LOGI(TAG, "resetting");
    ESP_LOGI(TAG, "resetting");
    ESP_LOGI(TAG, "resetting");
    ESP_LOGI(TAG, "resetting");
    ESP_LOGI(TAG, "resetting");
    ESP_LOGI(TAG, "resetting");
    ESP_LOGI(TAG, "resetting");
    ESP_LOGI(TAG, "resetting");
    ESP_LOGI(TAG, "resetting");
    ESP_LOGI(TAG, "resetting");
    ESP_LOGI(TAG, "resetting");
    ESP_LOGI(TAG, "resetting");
    ESP_LOGI(TAG, "resetting");
    ESP_LOGI(TAG, "resetting");
    ESP_LOGI(TAG, "resetting");
    ESP_LOGI(TAG, "resetting");
    ESP_LOGI(TAG, "resetting");
    ESP_LOGI(TAG, "resetting");
    ESP_LOGI(TAG, "resetting");
    ESP_LOGI(TAG, "resetting");
    ESP_LOGI(TAG, "resetting");
    ESP_LOGI(TAG, "resetting");
    wifi_prov_mgr_reset_provisioning();
    esp_restart();
}

static void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "time.google.com");
    sntp_setservername(1, "pool.ntp.org");
    sntp_init();
}

void obtain_time()
{
    initialize_sntp();
    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = {0};
    while (timeinfo.tm_year < (2016 - 1900))
    {
        ESP_LOGI(TAG, "Waiting for system time to be set...");
        gpio_num_t pins[2] = {BUTTON_LED};
        indicate(pins, 1, 1000, 1000, 1);
        time(&now);
        localtime_r(&now, &timeinfo);
    }
    ESP_LOGI(TAG, "Time is set...");
}

void output_pin_setup(gpio_num_t pin)
{
    gpio_reset_pin(pin);
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
}

void pin_setup()
{
    ESP_LOGI(TAG, "Setting up pins");
    output_pin_setup(BUTTON_BUZZER);
    output_pin_setup(BUTTON_LED);
    gpio_num_t pins[2] = {BUTTON_BUZZER, BUTTON_LED};
    indicate(pins, 2, 200, 200, 2);
    ESP_LOGI(TAG, "End of Setting up pins");

    button_handle_t reset_button;
    reset_button = iot_button_create(BUTTON_FOUR, BUTTON_ACTIVE_LOW); // this is button is purposefully set to low due to working mechanishm for the below used callback registration
    ESP_ERROR_CHECK_WITHOUT_ABORT(iot_button_add_on_press_cb(reset_button, 10, (button_cb)reset_device, NULL));
    // ESP_ERROR_CHECK_WITHOUT_ABORT(iot_button_set_evt_cb(reset_button, BUTTON_CB_TAP, (button_cb)reset_device, NULL));
}

esp_err_t button_setup()
{
    xQueue = xQueueCreate(512, 12 * sizeof(char));
    ESP_LOGI(TAG, "Setting up buttons");
    // initializing button and adding callback
    button_handle_t button_one;
    button_one = iot_button_create(BUTTON_ONE, BUTTON_ACTIVE_HIGH);
    ESP_ERROR_CHECK_WITHOUT_ABORT(iot_button_set_evt_cb(button_one, BUTTON_CB_TAP, (button_cb)push_button_callback, 1));
    // button two
    // initializing button and adding callback
    button_handle_t button_two;
    button_two = iot_button_create(BUTTON_TWO, BUTTON_ACTIVE_HIGH);
    ESP_ERROR_CHECK_WITHOUT_ABORT(iot_button_set_evt_cb(button_two, BUTTON_CB_TAP, (button_cb)push_button_callback, 2));
    ESP_LOGI(TAG, "end of Setting up buttons");

    // initializing button and adding callback
    button_handle_t button_three;
    button_three = iot_button_create(BUTTON_THREE, BUTTON_ACTIVE_HIGH);
    ESP_ERROR_CHECK_WITHOUT_ABORT(iot_button_set_evt_cb(button_three, BUTTON_CB_TAP, (button_cb)push_button_callback, 3));
    ESP_LOGI(TAG, "end of Setting up buttons");

    // initializing button and adding callback
    button_handle_t button_four;
    button_four = iot_button_create(BUTTON_FOUR, BUTTON_ACTIVE_HIGH);
    ESP_ERROR_CHECK_WITHOUT_ABORT(iot_button_set_evt_cb(button_four, BUTTON_CB_TAP, (button_cb)push_button_callback, 4));
    ESP_LOGI(TAG, "end of Setting up buttons");

    // initializing button and adding callback
    button_handle_t button_five;
    button_five = iot_button_create(BUTTON_FIVE, BUTTON_ACTIVE_HIGH);
    ESP_ERROR_CHECK_WITHOUT_ABORT(iot_button_set_evt_cb(button_five, BUTTON_CB_TAP, (button_cb)push_button_callback, 5));
    ESP_LOGI(TAG, "end of Setting up buttons");
    return ESP_OK;
}