/**
 * @file rainbow_led.h
 * @brief Rainbow LED driver
 */

#pragma once

#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h>

/* Types *********************************************************************/

/**
 * @brief Rainbow LED state change flags
 */
typedef enum {
    RAINBOW_LED_STATE_CHANGE_POWER = (1 << 0),
    RAINBOW_LED_STATE_CHANGE_SPEED = (1 << 1),
    RAINBOW_LED_STATE_CHANGE_BRIGHTNESS = (1 << 2),
    RAINBOW_LED_STATE_CHANGE_ALL = RAINBOW_LED_STATE_CHANGE_POWER | RAINBOW_LED_STATE_CHANGE_SPEED | RAINBOW_LED_STATE_CHANGE_BRIGHTNESS,
} rainbow_led_state_change_t;

/**
 * @brief Rainbow LED state
 */
typedef struct {
    bool power;                        // Power on/off.
    uint8_t speed;                     // Cycling speed. (1 - 10)
    uint8_t brightness;                // Brightness. (0 - 100)
    rainbow_led_state_change_t change; // State change flags.
} rainbow_led_state_t;

/**
 * @brief Rainbow LED callback. Called when the state changes.
 */
typedef void (*rainbow_led_callback_t)(const rainbow_led_state_t *state);

/**
 * @brief Initialize the rainbow LED.
 * @param initial_state Initial state.
 * @param callback Callback function.
 * @return ESP_OK on success, otherwise an error code.
 */
esp_err_t rainbow_led_init(const rainbow_led_state_t *initial_state, const rainbow_led_callback_t callback);

/**
 * @brief Set the state of the rainbow LED.
 * @param state New state. Mark the changed fields in the state change flags.
 *
 * e.g., to change the speed and brightness, set the change flags to RAINBOW_LED_STATE_CHANGE_SPEED | RAINBOW_LED_STATE_CHANGE_BRIGHTNESS.
 * to change only the power, set the change flags to RAINBOW_LED_STATE_CHANGE_POWER.
 * to change all the fields, set the change flags to RAINBOW_LED_STATE_CHANGE_ALL.
 * Make sure to set all the fields that are marked changed in the state change flags.
 *
 * @return ESP_OK on success, otherwise an error code.
 */
esp_err_t rainbow_led_set_state(const rainbow_led_state_t *state);