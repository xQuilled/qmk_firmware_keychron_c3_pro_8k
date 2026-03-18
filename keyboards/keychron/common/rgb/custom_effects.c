#include "quantum.h"
#include "rgb_matrix.h"
#include "color.h"
#include "custom_effects_helpers.h"

#if defined(KEYCHRON_RGB_ENABLE) && defined(EECONFIG_SIZE_CUSTOM_RGB)

/*
 * HALF_* effects are currently LED-index based: they split by LED index.
 * If you want physical "left half" vs "right half", split on kc_rgb_norm_x_u8(i).
 */

static void set_half_split_solid(effect_params_t *params, HSV color_hsv) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    const uint8_t half = RGB_MATRIX_LED_COUNT / 2;
    RGB           rgb  = hsv_to_rgb(color_hsv);

    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();

        if (i < half) {
            rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
        } else {
            rgb_matrix_set_color(i, 0, 0, 0);
        }
    }
}

bool HALF_PURPLE(effect_params_t *params) {
    // Purple in HSV on QMK's 0-255 hue wheel.
    set_half_split_solid(params, (HSV){.h = 191, .s = 255, .v = 255});
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    return rgb_matrix_check_finished_leds(led_max);
}

bool HALF_BLUE(effect_params_t *params) {
    // Blue in HSV on QMK's 0-255 hue wheel (tweak hue to taste).
    set_half_split_solid(params, (HSV){.h = 170, .s = 255, .v = 255});
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    return rgb_matrix_check_finished_leds(led_max);
}

/*
 * WHITE_WAVE:
 * A moving white "band" that sweeps left-to-right using the physical X coordinate map.
 *
 * Notes:
 * - Physical (x-based), not LED-index based.
 * - Uses the global "value" (brightness) from rgb_matrix_config.hsv.v.
 * - Speed is affected by rgb_matrix_config.speed.
 */
bool WHITE_WAVE(effect_params_t *params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    // Half-width of the bright band on the normalized X axis (0..255).
    // Larger => wider wave.
    const uint8_t half_width = 28;

    // Advance band center over time on a 0..255 axis.
    // qadd8(speed,1) avoids division by 0 and keeps slow speeds moving.
    const uint8_t t      = scale16by8(g_rgb_timer, qadd8(rgb_matrix_config.speed * 0.2, 1));
    const uint8_t center = t;

    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();

        const uint8_t x = kc_rgb_norm_x_u8(i);

        // Local falloff (0..255), then scaled by global brightness.
        const uint8_t local_v = kc_rgb_band_falloff_u8(x, center, half_width);
        const uint8_t v       = kc_rgb_apply_global_v(local_v);

        RGB rgb = kc_rgb_rgb_white(v);
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }

    return rgb_matrix_check_finished_leds(led_max);
}

#endif // defined(KEYCHRON_RGB_ENABLE) && defined(EECONFIG_SIZE_CUSTOM_RGB)
