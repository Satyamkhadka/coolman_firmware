
#include "esp_err.h"
#include "esp_log.h"
#include "iot_button.h"
#include "settings.h"
#include "button_cb.h"

#include <time.h>
#include "esp_sntp.h"
const static char *TAG = "button_setup";

// button  config
button_config_t button_one_config = {
    .type = BUTTON_TYPE_GPIO,
    .long_press_time = CONFIG_BUTTON_LONG_PRESS_TIME_MS,
    .short_press_time = CONFIG_BUTTON_SHORT_PRESS_TIME_MS,
    .gpio_button_config = {
        .gpio_num = BUTTON_ONE,
        .active_level = 0}};
button_config_t button_two_config = {.type = BUTTON_TYPE_GPIO, .long_press_time = CONFIG_BUTTON_LONG_PRESS_TIME_MS, .short_press_time = CONFIG_BUTTON_SHORT_PRESS_TIME_MS, .gpio_button_config = {.gpio_num = BUTTON_TWO, .active_level = 0}};
button_config_t button_three_config = {.type = BUTTON_TYPE_GPIO, .long_press_time = CONFIG_BUTTON_LONG_PRESS_TIME_MS, .short_press_time = CONFIG_BUTTON_SHORT_PRESS_TIME_MS, .gpio_button_config = {.gpio_num = BUTTON_THREE, .active_level = 0}};
button_config_t button_four_config = {.type = BUTTON_TYPE_GPIO, .long_press_time = CONFIG_BUTTON_LONG_PRESS_TIME_MS, .short_press_time = CONFIG_BUTTON_SHORT_PRESS_TIME_MS, .gpio_button_config = {.gpio_num = BUTTON_FOUR, .active_level = 0}};
button_config_t button_five_config = {.type = BUTTON_TYPE_GPIO, .long_press_time = CONFIG_BUTTON_LONG_PRESS_TIME_MS, .short_press_time = CONFIG_BUTTON_SHORT_PRESS_TIME_MS, .gpio_button_config = {.gpio_num = BUTTON_FIVE, .active_level = 0}};

static void
initialize_sntp(void)
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
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }
    ESP_LOGI(TAG, "Time is set...");
}

void pin_setup()
{
    ESP_LOGI(TAG, "Setting up buzzer pins");
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
    xQueue = xQueueCreate(512, 12 * sizeof(char));
    ESP_LOGI(TAG, "Setting up buttons BUTTON_ONE");
    // initializing button and adding callback
    button_handle_t button_one;
    button_one = iot_button_create(&button_one_config);
    iot_button_register_cb(button_one, BUTTON_SINGLE_CLICK, push_button_callback, (void *)1);
    // button two
    // initializing button and adding callback
    button_handle_t button_two;
    button_two = iot_button_create(&button_two_config);
    iot_button_register_cb(button_two, BUTTON_SINGLE_CLICK, push_button_callback, (void *)2);
    ESP_LOGI(TAG, "end of Setting up buttons BUTTON_TWO");

    // initializing button and adding callback
    button_handle_t button_three;
    button_three = iot_button_create(&button_three_config);
    iot_button_register_cb(button_three, BUTTON_SINGLE_CLICK, push_button_callback, (void *)3);
    ESP_LOGI(TAG, "end of Setting up buttons BUTTON_THREE");

    // initializing button and adding callback
    button_handle_t button_four;
    button_four = iot_button_create(&button_four_config);
    iot_button_register_cb(button_four, BUTTON_SINGLE_CLICK, push_button_callback, (void *)4);
    ESP_LOGI(TAG, "end of Setting up buttons BUTTON_FOUR");

    // initializing button and adding callback
    button_handle_t button_five;
    button_five = iot_button_create(&button_five_config);
    iot_button_register_cb(button_five, BUTTON_SINGLE_CLICK, push_button_callback, (void *)5);
    ESP_LOGI(TAG, "end of Setting up buttons BUTTON_FIVE");
    return ESP_OK;
}