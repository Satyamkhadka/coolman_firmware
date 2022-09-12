#include "indicator.h"

void buzzer_setup()
{
    const blink_step_t test_blink_one_time[] = {
        {LED_BLINK_HOLD, LED_STATE_ON, 50},   // step1: turn on LED 50 ms
        {LED_BLINK_HOLD, LED_STATE_OFF, 100}, // step2: turn off LED 100 ms
        {LED_BLINK_HOLD, LED_STATE_ON, 150},  // step3: turn on LED 150 ms
        {LED_BLINK_HOLD, LED_STATE_OFF, 100}, // step4: turn off LED 100 ms
        {LED_BLINK_STOP, 0, 0},               // step5: stop blink (off)
    };

    const blink_step_t test_blink_loop[] = {
        {LED_BLINK_HOLD, LED_STATE_ON, 50},   // step1: turn on LED 50 ms
        {LED_BLINK_HOLD, LED_STATE_OFF, 100}, // step2: turn off LED 100 ms
        {LED_BLINK_LOOP, 0, 0},               // step3: loop from step1
    };

    typedef enum
    {
        BLINK_TEST_BLINK_ONE_TIME, /**< test_blink_one_time */
        BLINK_TEST_BLINK_LOOP,     /**< test_blink_loop */
        BLINK_MAX,                 /**< INVALIED type */
    };

    blink_step_t const *led_indicator_blink_lists[] = {
        [BLINK_TEST_BLINK_ONE_TIME] = test_blink_one_time,
        [BLINK_TEST_BLINK_LOOP] = test_blink_loop,
        [BLINK_MAX] = NULL,
    };

    led_indicator_config_t config = {
        .off_level = 0, // attach led positive side to esp32 gpio pin
        .mode = LED_GPIO_MODE,
    };
    led_indicator_handle_t led_handle = led_indicator_create(6, &config); // attach to gpio 8

    led_indicator_start(led_handle, BLINK_TEST_BLINK_LOOP); // call to start, the function does not block
}

// led_indicator_stop(led_handle, BLINK_TEST_BLINK_LOOP); // call stop

// led_indicator_delete(&led_handle);