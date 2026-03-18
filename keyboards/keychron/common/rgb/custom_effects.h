/* Copyright 2026
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Forward declarations for custom RGB Matrix effects implemented in
 * keyboards/keychron/common/rgb/custom_effects.c.
 */

#pragma once

#include "rgb_matrix.h"

#if defined(KEYCHRON_RGB_ENABLE) && defined(EECONFIG_SIZE_CUSTOM_RGB)

bool HALF_PURPLE(effect_params_t *params);
bool HALF_BLUE(effect_params_t *params);

#endif
