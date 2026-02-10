/**
 * @file rainbow_led.c
 * @brief Rainbow LED driver
 */

/* Includes ******************************************************************/

/* Declarations */
#include "rainbow_led.h"

/* Hardware drivers */
#include "app_led.h"
#include "app_button.h"

/* ESP-IDF libraries */
#include "esp_log.h"
#include "sdkconfig.h"

/* FreeRTOS libraries */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

/* Standard libraries */
#include <time.h>
#include <stdlib.h>
#include <inttypes.h>

/* Types *********************************************************************/

/**
 * @brief Private data for the rainbow LED
 */
typedef struct {
    rainbow_led_state_t state;
    rainbow_led_callback_t callback;
    EventGroupHandle_t event_group;
} rainbow_led_priv_data_t;

/**
 * @brief Range for the rainbow LED parameters
 */
typedef struct {
    uint8_t max;
    uint8_t min;
} rainbow_led_range_t;

/* Constants *****************************************************************/

/**
 * @brief Tag for logging
 */
static const char *TAG = "rainbow_led";

/**
 * @brief Stack size for the rainbow LED task
 */
#define RAINBOW_LED_TASK_STACK_SIZE CONFIG_RAINBOW_LED_TASK_STACK_SIZE

/**
 * @brief Priority for the rainbow LED task
 */
#define RAINBOW_LED_TASK_PRIORITY CONFIG_RAINBOW_LED_TASK_PRIORITY

/**
 * @brief Event group bit for the power
 */
#define RAINBOW_LED_EVENT_GROUP_BIT_POWER (1 << 0)

static const struct {
    rainbow_led_range_t speed;
    rainbow_led_range_t brightness;
} _ranges = {
    .speed = {
        .max = 10,
        .min = 1,
    },
    .brightness = {
        .max = 100,
        .min = 0,
    },
};
/* Variables *****************************************************************/

/**
 * @brief Private data for the rainbow LED
 */
static rainbow_led_priv_data_t _data = {
    .event_group = NULL,
    .state = {
        .power = false,
        .speed = _ranges.speed.min,
        .brightness = _ranges.brightness.min,
    },
    .callback = NULL,
};
/* Private Functions **********************************************************/

/**
 * @brief Task function for the rainbow LED
 */
static void _task_function(void *unused)
{
    (void)unused;

    ESP_LOGI(TAG, "Rainbow LED task started");

    app_led_color_hsv_t color = {
        .hue = 0,
        .saturation = 100,
        .brightness = _data.state.brightness,
    };

    while (1) {
        if (!_data.state.power) {
            EventBits_t bits = xEventGroupWaitBits(_data.event_group, RAINBOW_LED_EVENT_GROUP_BIT_POWER, pdTRUE, pdTRUE, portMAX_DELAY);
            if (!(bits & RAINBOW_LED_EVENT_GROUP_BIT_POWER)) {
                continue;
            }
        }

        /* Increment the hue by the speed. */
        color.hue += _data.state.speed;
        if (color.hue >= 360) {
            color.hue -= 360;
        }

        /* Set the brightness. */
        color.brightness = _data.state.brightness;
        
        /* Set the color. */
        app_led_set_color_hsv(color);
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

static void _on_power_toggle(bool new_power)
{
    _data.state.power = new_power;
    app_led_set_power(new_power);
    if (new_power) {
        xEventGroupSetBits(_data.event_group, RAINBOW_LED_EVENT_GROUP_BIT_POWER);
    } else {
        xEventGroupClearBits(_data.event_group, RAINBOW_LED_EVENT_GROUP_BIT_POWER);
    }
}

static void _on_short_press(void *unused1, void *unused2)
{
    _on_power_toggle(!_data.state.power);
    rainbow_led_state_t new_state = _data.state;
    new_state.change = RAINBOW_LED_STATE_CHANGE_POWER;
    _data.callback(&new_state);
}

/**
 * @brief Callback function for the long press of the button
 * This function will randomize the speed and brightness of the LED.
 */
static void _on_long_press(void *unused1, void *unused2)
{
    rainbow_led_state_t new_state = _data.state;
    new_state.speed = rand() % (_ranges.speed.max - _ranges.speed.min + 1) + _ranges.speed.min;
    new_state.brightness = rand() % (_ranges.brightness.max - _ranges.brightness.min + 1) + _ranges.brightness.min;
    new_state.change = RAINBOW_LED_STATE_CHANGE_SPEED | RAINBOW_LED_STATE_CHANGE_BRIGHTNESS;
    _data.callback(&new_state);
}

/* Public Functions **********************************************************/

esp_err_t rainbow_led_init(const rainbow_led_state_t *initial_state, const rainbow_led_callback_t callback)
{
    if (initial_state == NULL || callback == NULL) {
        ESP_LOGE(TAG, "Invalid arguments: initial_state = %p, callback = %p", initial_state, callback);
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret = ESP_OK;

    // Initialize RNG.
    srand(time(NULL));

    // Initialize the event group.
    _data.event_group = xEventGroupCreate();
    if (_data.event_group == NULL) {
        ESP_LOGE(TAG, "Failed to create the event group");
        return ESP_ERR_NO_MEM;
    }

    // Initialize the LED.
    ret = app_led_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize the LED: %d", ret);
        return ret;
    }

    // Initialize the button.
    app_button_config_t button_config = {
        .callbacks = {
            .on_short_press = _on_short_press,
            .on_long_press = _on_long_press,
        },
    };
    ret = app_button_init(&button_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize the button: %d", ret);
        return ret;
    }

    _data.state = *initial_state;
    _data.callback = callback;
    app_led_set_power(_data.state.power);
    xTaskCreate(_task_function, "rainbow_led", RAINBOW_LED_TASK_STACK_SIZE, NULL, RAINBOW_LED_TASK_PRIORITY, NULL);
    return ESP_OK;
}

esp_err_t rainbow_led_set_state(const rainbow_led_state_t *state)
{
    /* Check the arguments. */
    if (state == NULL) {
        ESP_LOGE(TAG, "Invalid arguments: state = %p", state);
        return ESP_ERR_INVALID_ARG;
    }
    uint8_t change = state->change;
    if ((change & RAINBOW_LED_STATE_CHANGE_SPEED) && (state->speed < _ranges.speed.min || state->speed > _ranges.speed.max)) {
        ESP_LOGE(TAG, "Invalid speed: %" PRIu16 " (min = %" PRIu16 ", max = %" PRIu16 ")", (uint16_t)state->speed, (uint16_t)_ranges.speed.min, (uint16_t)_ranges.speed.max);
        return ESP_ERR_INVALID_ARG;
    }
    if ((change & RAINBOW_LED_STATE_CHANGE_BRIGHTNESS) && (state->brightness < _ranges.brightness.min || state->brightness > _ranges.brightness.max)) {
        ESP_LOGE(TAG, "Invalid brightness: %" PRIu16 " (min = %" PRIu16 ", max = %" PRIu16 ")", (uint16_t)state->brightness, (uint16_t)_ranges.brightness.min, (uint16_t)_ranges.brightness.max);
        return ESP_ERR_INVALID_ARG;
    }

    /* Update the state. */
    if (change & RAINBOW_LED_STATE_CHANGE_SPEED) {
        _data.state.speed = state->speed;
    }
    if (change & RAINBOW_LED_STATE_CHANGE_BRIGHTNESS) {
        _data.state.brightness = state->brightness;
    }
    if (change & RAINBOW_LED_STATE_CHANGE_POWER) {
        _on_power_toggle(state->power);
    }
    return ESP_OK;
}
