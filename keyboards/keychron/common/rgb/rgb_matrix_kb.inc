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

#include "rgb_matrix_kb_config.h"

#if defined(KEYCHRON_RGB_ENABLE) && defined(EECONFIG_SIZE_CUSTOM_RGB)

/*
 * QMK's RGB Matrix core includes this file in a pass where it defines
 * RGB_MATRIX_CUSTOM_EFFECT_IMPLS. During that pass this file expands into
 * switch/call sites for the custom effects, so we must provide prototypes to
 * avoid implicit function declarations (-Werror).
 *
 * This makes rgb_matrix_kb.inc the single source of truth:
 * - Effect list (RGB_MATRIX_EFFECT)
 * - Corresponding function prototypes
 */
#    ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS
extern bool HALF_PURPLE(effect_params_t *params);
extern bool HALF_BLUE(effect_params_t *params);
extern bool WHITE_WAVE(effect_params_t *params);
#    endif

RGB_MATRIX_EFFECT(HALF_PURPLE)
RGB_MATRIX_EFFECT(HALF_BLUE)
RGB_MATRIX_EFFECT(WHITE_WAVE)

#endif
