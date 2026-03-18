/* Copyright 2024 ~ 2025 @ Keychron (https://www.keychron.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include QMK_KEYBOARD_H
#include "raw_hid.h"
#include "keychron_common.h"
#include "keychron_rgb_type.h"
#include "eeconfig_kb.h"
#include "nvm_eeprom_eeconfig_internal.h"
#include "eeprom.h"
#include "usb_main.h"
#include "color.h"
#include "backlit_indicator.h"
#if defined(LK_WIRELESS_ENABLE) || defined(KC_BLUETOOTH_ENABLE)
#    include "transport.h"
#    include "wireless.h"
#endif
#include <lib/lib8tion/lib8tion.h>
#include "config.h"

#if defined(KEYCHRON_RGB_ENABLE) && defined(EECONFIG_SIZE_CUSTOM_RGB)

#    define PER_KEY_RGB_VER 0x0001

#    define OFFSET_OS_INDICATOR ((uint8_t *)(EECONFIG_BASE_CUSTOM_RGB))

enum {
    RGB_GET_PROTOCOL_VER = 0x01,
    RGB_SAVE,
    GET_INDICATORS_CONFIG,
    SET_INDICATORS_CONFIG,
    RGB_GET_LED_COUNT,
    RGB_GET_LED_IDX,
};

os_indicator_config_t os_ind_cfg;

void eeconfig_reset_custom_rgb(void) {
    // Only keep OS indicator config in custom RGB EEPROM.
    // Per-key RGB, mixed RGB, and retail demo storage have been removed from the build.
    os_ind_cfg.disable.raw = 0;
    os_ind_cfg.hsv.s       = 0;
    os_ind_cfg.hsv.h = os_ind_cfg.hsv.v = 0xFF;

    eeprom_update_block(&os_ind_cfg, OFFSET_OS_INDICATOR, sizeof(os_ind_cfg));
}

void eeconfig_init_custom_rgb(void) {
    eeprom_update_dword(EECONFIG_KEYBOARD, (EECONFIG_KB_DATA_VERSION));

    // Only load OS indicator config in custom RGB EEPROM.
    eeprom_read_block(&os_ind_cfg, OFFSET_OS_INDICATOR, sizeof(os_ind_cfg));

    if (os_ind_cfg.hsv.v < 128) os_ind_cfg.hsv.v = 128;
}

void rgb_save_retail_demo(void) {
    // Retail demo is not built; keep symbol for compatibility.
}

static bool rgb_get_version(uint8_t *data) {
    data[1] = PER_KEY_RGB_VER & 0xFF;
    data[2] = (PER_KEY_RGB_VER >> 8) & 0xFF;

    return true;
}

static bool rgb_get_led_count(uint8_t *data) {
    data[1] = RGB_MATRIX_LED_COUNT;

    return true;
}

static bool rgb_get_led_idx(uint8_t *data) {
    uint8_t row = data[0];
    if (row > MATRIX_ROWS) return false;

    uint8_t  led_idx[128];
    uint32_t row_mask = 0;
    memcpy(&row_mask, &data[1], 3);

    for (uint8_t c = 0; c < MATRIX_COLS; c++) {
        led_idx[0] = 0xFF;
        if (row_mask & (0x01 << c)) {
            rgb_matrix_map_row_column_to_led(row, c, led_idx);
        }
        data[1 + c] = led_idx[0];
    }

    return true;
}

// Per-key RGB and mixed RGB endpoints removed (effects not built).

// mixed_rgb_set_effect_list removed (mixed effects not built).

static bool kc_rgb_save(void) {
    // Only persist OS indicator config (other custom RGB data removed from build).
    eeprom_update_block(&os_ind_cfg, OFFSET_OS_INDICATOR, sizeof(os_ind_cfg));
    return true;
}

static bool get_indicators_config(uint8_t *data) {
    data[1] = 0
#    if defined(NUM_LOCK_INDEX) && !defined(DIM_NUM_LOCK)
              | (1 << 0x00)
#    endif
#    if defined(CAPS_LOCK_INDEX) && !defined(DIM_CAPS_LOCK)
              | (1 << 0x01)
#    endif
#    if defined(SCROLL_LOCK_INDEX)
              | (1 << 0x02)
#    endif
#    if defined(COMPOSE_LOCK_INDEX)
              | (1 << 0x03)
#    endif
#    if defined(KANA_LOCK_INDEX)
              | (1 << 0x04)
#    endif
        ;
    data[2] = os_ind_cfg.disable.raw;
    data[3] = os_ind_cfg.hsv.h;
    data[4] = os_ind_cfg.hsv.s;
    data[5] = os_ind_cfg.hsv.v;

    return true;
}

static bool set_indicators_config(uint8_t *data) {
    os_ind_cfg.disable.raw = data[0];
    os_ind_cfg.hsv.h       = data[1];
    os_ind_cfg.hsv.s       = data[2];
    os_ind_cfg.hsv.v       = data[3];

    if (os_ind_cfg.hsv.v < 128) os_ind_cfg.hsv.v = 128;
    led_update_kb(host_keyboard_led_state());

    return true;
}

void kc_rgb_matrix_rx(bool usb, uint8_t *data, uint8_t length) {
    (void)usb;
    (void)length;

    bool success = false;

    switch (data[1]) {
        case RGB_GET_PROTOCOL_VER:
            success = rgb_get_version(&data[2]);
            break;

        case RGB_SAVE:
            success = kc_rgb_save();
            break;

        case GET_INDICATORS_CONFIG:
            success = get_indicators_config(&data[2]);
            break;

        case SET_INDICATORS_CONFIG:
            success = set_indicators_config(&data[2]);
            break;

        case RGB_GET_LED_COUNT:
            success = rgb_get_led_count(&data[2]);
            break;

        case RGB_GET_LED_IDX:
            success = rgb_get_led_idx(&data[2]);
            break;

        /*
         * Per-key RGB and Mixed RGB raw HID protocol handlers intentionally removed.
         * Those modules are no longer built, and we don't want to expose those endpoints.
         */

        default:
            break;
    }

    // Keychron protocol expects a status byte at data[2]: 0 = OK, 1 = FAIL
    data[2] = success ? 0 : 1;
}

void os_state_indicate(void) {
#    if defined(RGB_MATRIX_SLEEP) || defined(LED_MATRIX_SLEEP)
#        if defined(LK_WIRELESS_ENABLE) || defined(KC_BLUETOOTH_ENABLE)
    if (get_transport() == TRANSPORT_USB && USB_DRIVER.state == USB_SUSPENDED) return;
#        else
    if (USB_DRIVER.state == USB_SUSPENDED) return;
#        endif
#    endif

#    if (defined(NUM_LOCK_INDEX) && !defined(DIM_NUM_LOCK)) || (defined(CAPS_LOCK_INDEX) && !defined(DIM_CAPS_LOCK)) || defined(SCROLL_LOCK_INDEX) || defined(COMPOSE_LOCK_INDEX) || defined(KANA_LOCK_INDEX)
    RGB rgb = hsv_to_rgb(os_ind_cfg.hsv);
#    endif

#    if defined(NUM_LOCK_INDEX)
    if (host_keyboard_led_state().num_lock && !os_ind_cfg.disable.num_lock) {
#        if defined(DIM_NUM_LOCK)
        SET_LED_OFF(DIM_NUM_LOCK);
#        else
        rgb_matrix_set_color(NUM_LOCK_INDEX, rgb.r, rgb.g, rgb.b);
#        endif
    }
#    endif
#    if defined(CAPS_LOCK_INDEX)
    if (host_keyboard_led_state().caps_lock && !os_ind_cfg.disable.caps_lock) {
#        if defined(DIM_CAPS_LOCK)
        SET_LED_OFF(CAPS_LOCK_INDEX);
#        else
        rgb_matrix_set_color(CAPS_LOCK_INDEX, rgb.r, rgb.g, rgb.b);
#        endif
    }
#    endif
#    if defined(SCROLL_LOCK_INDEX)
    if (host_keyboard_led_state().compose && !os_ind_cfg.disable.scroll_lock) {
        rgb_matrix_set_color(SCROLL_LOCK_INDEX, rgb.r, rgb.g, rgb.b);
    }
#    endif
#    if defined(COMPOSE_LOCK_INDEX)
    if (host_keyboard_led_state().compose && !os_ind_cfg.disable.compose) {
        rgb_matrix_set_color(COMPOSE_LOCK_INDEX, rgb.r, rgb.g, rgb.b);
    }
#    endif
#    if defined(KANA_LOCK_INDEX)
    if (host_keyboard_led_state().kana && !os_ind_cfg.disable.kana) {
        rgb_matrix_set_color(KANA_LOCK_INDEX, rgb.r, rgb.g, rgb.b);
    }
#    endif
#    if defined(WINLOCK_LED_LIST) || defined(WIN_LOCK_LED_PIN)
    // TODO: check if we can use (get_transport() == TRANSPORT_USB || wireless_get_state() == WT_CONNECTED)

    if (1
#        if defined(LK_WIRELESS_ENABLE) || defined(KC_BLUETOOTH_ENABLE)
        && (get_transport() == TRANSPORT_USB || ((get_transport() & TRANSPORT_WIRELESS) && wireless_get_state() == WT_CONNECTED))
#        endif
    ) {
#        ifdef WIN_BASE_LAYER
        if (get_highest_layer(default_layer_state) == WIN_BASE_LAYER)
#        endif
        {
#        if defined(WINLOCK_LED_LIST)
            uint8_t idx_list[] = WINLOCK_LED_LIST;
            for (uint8_t i = 0; i < sizeof(idx_list); i++) {
                if (keymap_config.no_gui) {
#            ifdef DIM_WIN_LOCK
                    SET_LED_OFF(idx_list[i]);
#            else
                    rgb_matrix_set_color(idx_list[i], 255, 0, 0);
#            endif
                }
            }
#        endif
#        if defined(WIN_LOCK_LED_PIN)
            gpio_set_pin_output_push_pull(WIN_LOCK_LED_PIN);
            gpio_write_pin(WIN_LOCK_LED_PIN, keymap_config.no_gui ? WIN_LOCK_LED_PIN_ON_STATE : !WIN_LOCK_LED_PIN_ON_STATE);
#        endif
        }
#        if defined(WIN_BASE_LAYER) && defined(WIN_LOCK_LED_PIN)
        else {
            gpio_set_pin_output_push_pull(WIN_LOCK_LED_PIN);
            gpio_write_pin(WIN_LOCK_LED_PIN, !WIN_LOCK_LED_PIN_ON_STATE);
        }
#        endif
    }
#    endif
}

void rgb_matrix_none_indicators(void) {
    os_state_indicate();
    rgb_matrix_none_indicators_kb();
    rgb_matrix_none_indicators_user();
}

bool process_record_keychron_rgb(uint16_t keycode, keyrecord_t *record) {
    // We no longer use Keychron's per-key/mixed effect stack, so don't intercept
    // underglow/rgb-matrix shared keys. Let QMK handle UG_* (including UG_NEXT/UG_PREV)
    // so it can cycle through custom modes like HALF_PURPLE/HALF_BLUE.
    (void)keycode;
    (void)record;
    return true;
}
#endif
