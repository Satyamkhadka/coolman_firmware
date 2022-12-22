

#define MAX_BUFFER_TO_STORE 1
#define MAX_RECORD_TO_SEND_AT_ONCE 10
#include "freertos/semphr.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

esp_err_t send_data(const char *data);
void https_send_task();
