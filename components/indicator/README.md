## LED indicator

LED indicator is one of the simplest output 外成, it can indicate the current working state of the system through flashing in different forms. The LED indicator component provided by ESP-IoT-Solution has the following functions:

* Support definition 多组内容引语
* Support definition scintillation type priority
* Support for creating multiple indicators

### Usage method

###

Blinking step structure blink_step_t defines the type of the step, indicator state, and status duration. Multiple steps are combined into one blinking type, and different blinking types can be used to identify different system states. The definition method of blinking type is as follows:

Example 1. Define one cycle flashing：亮 0.05 S，亮 0.1 S，beginning after continuous cycle。

```
const blink_step_t test_blink_loop[] = {
    {LED_BLINK_HOLD, LED_STATE_ON, 50}, // step1: turn on LED 50 ms
    {LED_BLINK_HOLD, LED_STATE_OFF, 100}, // step2: turn off LED 100 ms
    {LED_BLINK_LOOP, 0, 0}, // step3: loop from step1
};
```

Example 2. Definition of one cycle flashing：亮 0.05 S，亮 0.1 S，亮 0.15 S，亮 0.1 S，电影完毕灯熳线。

```
const blink_step_t test_blink_one_time[] = {
    {LED_BLINK_HOLD, LED_STATE_ON, 50}, // step1: turn on LED 50 ms
    {LED_BLINK_HOLD, LED_STATE_OFF, 100}, // step2: turn off LED 100 ms
    {LED_BLINK_HOLD, LED_STATE_ON, 150}, // step3: turn on LED 150 ms
    {LED_BLINK_HOLD, LED_STATE_OFF, 100}, // step4: turn off LED 100 ms
    {LED_BLINK_STOP, 0, 0}, // step5: stop blink (off)
};
```

After defining the blink type, you need to add `led_indicator_blink_type_t` to the corresponding member, then add it to the `led_indicator_blink_lists` list, the example is as follows:

```
typedef enum {
    BLINK_TEST_BLINK_ONE_TIME, /**< test_blink_one_time */
    BLINK_TEST_BLINK_LOOP, /**< test_blink_loop */
    BLINK_MAX, /**< INVALIED type */
} led_indicator_blink_type_t;

blink_step_t const * led_indicator_blink_lists[] = {
    [BLINK_TEST_BLINK_ONE_TIME] = test_blink_one_time,
    [BLINK_TEST_BLINK_LOOP] = test_blink_loop,
    [BLINK_MAX] = NULL,
};
```

###

For the same indicator, high-priority blinking can be interrupted by low-priority blinking, when the high-priority blinking ends, low-priority blinking resumes.的 priority level, the value of the smaller members will execute the higher priority level.

For example, in the following example, the blinking test_blink_one_time is higher than test_blink_loop, it can be blinked first.

```
typedef enum {
    BLINK_TEST_BLINK_ONE_TIME, /**< test_blink_one_time */
    BLINK_TEST_BLINK_LOOP, /**< test_blink_loop */
    BLINK_MAX, /**< INVALIED type */
} led_indicator_blink_type_t;
```

#### Control indicator flashes

Create an indicator: specify a single IO and set a group of configuration information to create an indicator

```
led_indicator_config_t config = {
    .off_level = 0, // attach led positive side to esp32 gpio pin
    .mode = LED_GPIO_MODE,
};
led_indicator_handle_t led_handle = led_indicator_create(8, &config); // attach to gpio 8
```

Start/stop flashing：Control the indicator light to open/stop the specified flashing type, return immediately after the function is called, internally by the timer control flashing process. The same indicator can turn on multiple types of flashing, according to the priority of the flashing.

```
led_indicator_start(led_handle, BLINK_TEST_BLINK_LOOP); // call to start, the function does not block

/*
*......
*/

led_indicator_stop(led_handle, BLINK_TEST_BLINK_LOOP); // call stop
```

Delete the indicator: You can also delete the indicator when you don't need further operation

```
led_indicator_delete(&led_handle);
```

> The component supports thread safety operation, you can use global variable sharing LED indicator operation 句柄 led_indicator_handle_t, you can also use led_indicator_get_handle on other thread through LED's IO 号 get 句柄以通过数意.