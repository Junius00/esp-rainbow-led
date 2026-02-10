/**
 * @file example.c
 * @brief Example application for the rainbow LED.
 */

/* Includes ******************************************************************/

/* Declarations */
#include "rainbow_led.h"

/* ESP-IDF libraries */
#include "esp_log.h"

/* Constants *****************************************************************/

/**
 * @brief Tag for logging
 */
static const char *TAG = "rainbow-led-example";

/* Variables *****************************************************************/

/* Private Functions **********************************************************/

/**
 * @brief Callback function for the rainbow LED.
 */
static void _on_rainbow_led_state_change(const rainbow_led_state_t *state)
{
    /* Detect the changes. */
    if (state->change & RAINBOW_LED_STATE_CHANGE_POWER) {
        ESP_LOGI(TAG, "[CHANGE] Power: %s", state->power ? "ON" : "OFF");
    }
    if (state->change & RAINBOW_LED_STATE_CHANGE_SPEED) {
        ESP_LOGI(TAG, "[CHANGE] Speed: %d", state->speed);
    }
    if (state->change & RAINBOW_LED_STATE_CHANGE_BRIGHTNESS) {
        ESP_LOGI(TAG, "[CHANGE] Brightness: %d", state->brightness);
    }
}

/* Functions *****************************************************************/

/**
 * @brief Main function for the example application.
 */
void app_main(void)
{
    /* Initialize the rainbow LED. */
    rainbow_led_state_t initial_state = {
        .power = true,
        .speed = 5,
        .brightness = 20,
    };
    rainbow_led_init(&initial_state, _on_rainbow_led_state_change);

    ESP_LOGI(TAG, "Rainbow LED initialized. Use the BOOT button to control the LED.");
}
