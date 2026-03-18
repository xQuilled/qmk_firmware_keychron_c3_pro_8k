OPT_DEFS += -DKEYCHRON_RGB_ENABLE

RGB_MATRIX_CUSTOM_KB = yes
RGB_MATRIX_DIR = $(TOP_DIR)/keyboards/keychron/common/rgb

SRC += \
     $(RGB_MATRIX_DIR)/keychron_rgb.c \
     $(RGB_MATRIX_DIR)/custom_effects.c

VPATH += $(RGB_MATRIX_DIR)
