# rainbow-led

This is an implementation of a rainbow LED with speed and brightness control. The BOOT button can be used to control:
- **Short press**: Power toggle
- **Long press**: Random speed and brightness

## Example

An example project is included under [`examples`](./examples/get_started).

## Configuration

### LED

When configuring your project, be sure to select:
- **LED module type**: `CONFIG_APP_LED_TYPE`
    - LEDC: `CONFIG_APP_LED_TYPE_LEDC=y`
    - WS2812: `CONFIG_APP_LED_TYPE_WS2812=y` (default)
- **GPIO pins**:
    - LEDC: `APP_LED_LEDC_GPIO_NUM_[R/G/B]` (default: R=0, G=1, B=8)
    - WS2812: `APP_LED_WS2812_GPIO_NUM=?` (default: 8)

Or if using `idf.py menuconfig`, then head to `Component config > App LED Control`.

### Button

If your BOOT button is non-standard, or using a different button, you should configure:
- **GPIO pin**: `CONFIG_APP_BUTTON_GPIO_NUM=?` (default: 9)
- **Active level**: `CONFIG_APP_BUTTON_IS_ACTIVE_HIGH=[y/n]` (default: y)

Or if using `idf.py menuconfig`, then head to `Component config > App Button Configuration`.
