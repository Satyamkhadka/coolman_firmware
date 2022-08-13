
#include "esp_err.h"
#include "esp_log.h"
#include "iot_button.h"
#include "settings.h"
esp_err_t button_setup()
{
    // initializing button and adding callback
    button_handle_t charge_rate_button;
    charge_rate_button = iot_button_create(BUTTON_RESET, BUTTON_ACTIVE_HIGH);
    // ESP_ERROR_CHECK_WITHOUT_ABORT(iot_button_set_evt_cb(charge_rate_button, BUTTON_CB_TAP, (button_cb)NULL, NULL));
    return ESP_OK;
}