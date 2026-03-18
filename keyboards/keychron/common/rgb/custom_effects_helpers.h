/* Copyright 2026
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Reusable helpers for custom RGB Matrix effects in the Keychron fork.
 *
 * Goals:
 * - Keep effect code small and readable.
 * - Provide basic color helpers (white HSV/RGB).
 * - Provide physical X normalization (0..255) using g_led_config.point[].x.
 * - Provide a simple "band falloff" function to build waves/sweeps.
 */

#pragma once

#include "rgb_matrix.h"
#include "color.h"
#include <lib/lib8tion/lib8tion.h>

#if defined(KEYCHRON_RGB_ENABLE) && defined(EECONFIG_SIZE_CUSTOM_RGB)

/* ---------- Color helpers ---------- */

/* White in HSV is simply saturation = 0. Hue doesn't matter; we use 0 for clarity. */
static inline HSV kc_rgb_hsv_white(uint8_t v) {
    return (HSV){.h = 0, .s = 0, .v = v};
}

static inline RGB kc_rgb_rgb_white(uint8_t v) {
    return hsv_to_rgb(kc_rgb_hsv_white(v));
}

/* Scale an 8-bit value by the current RGB Matrix global brightness (value). */
static inline uint8_t kc_rgb_apply_global_v(uint8_t local_v) {
    return scale8(local_v, rgb_matrix_config.hsv.v);
}

/* ---------- Physical coordinate helpers ---------- */

/*
 * Returns a normalized physical X coordinate in the range [0..255] for LED index i.
 *
 * Notes:
 * - Uses g_led_config.point[i].x, so it reflects the keyboard's physical LED map.
 * - If x_max == x_min (degenerate), returns 0.
 */
static inline uint8_t kc_rgb_norm_x_u8(uint8_t i) {
    uint8_t x_min = 255;
    uint8_t x_max = 0;

    for (uint8_t j = 0; j < RGB_MATRIX_LED_COUNT; j++) {
        const uint8_t x = g_led_config.point[j].x;
        if (x < x_min) x_min = x;
        if (x > x_max) x_max = x;
    }

    const uint8_t x = g_led_config.point[i].x;
    if (x_max <= x_min) {
        return 0;
    }

    // Normalize to 0..255
    const uint16_t numer = (uint16_t)(x - x_min) * 255u;
    const uint8_t  denom = (uint8_t)(x_max - x_min);
    return (uint8_t)(numer / denom);
}

/* ---------- Band / wave helpers ---------- */

/*
 * Compute a soft "band" falloff around `center` on a 0..255 axis.
 *
 * Returns brightness in [0..255], where:
 * - dist == 0 => 255
 * - dist >= half_width => 0
 *
 * This is a simple linear falloff suitable for sweeps and waves.
 */
static inline uint8_t kc_rgb_band_falloff_u8(uint8_t pos, uint8_t center, uint8_t half_width) {
    if (half_width == 0) {
        return (pos == center) ? 255 : 0;
    }

    const uint8_t dist = (pos > center) ? (pos - center) : (center - pos);
    if (dist >= half_width) {
        return 0;
    }

    // dist: 0..half_width-1 => 255..(small)
    // Scale linearly: 0 => 255, half_width => 0
    // Use 16-bit to avoid truncation during multiplication.
    const uint16_t scaled = (uint16_t)(half_width - dist) * 255u;
    return (uint8_t)(scaled / half_width);
}

#endif // defined(KEYCHRON_RGB_ENABLE) && defined(EECONFIG_SIZE_CUSTOM_RGB)