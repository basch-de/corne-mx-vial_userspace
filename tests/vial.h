// EEPROM boundary for testing the unmodified Vial Tap Dance implementation.
#pragma once
#include "mock.h"
#define VIAL_TAP_DANCE_ENABLE
#define VIAL_TAP_DANCE_ENTRIES 4
typedef struct {
    uint16_t on_tap, on_hold, on_double_tap, on_tap_hold, custom_tapping_term;
} vial_tap_dance_entry_t;
int dynamic_keymap_get_tap_dance(uint8_t, vial_tap_dance_entry_t *);
int dynamic_keymap_set_tap_dance(uint8_t, const vial_tap_dance_entry_t *);
void vial_keycode_down(uint16_t);
void vial_keycode_up(uint16_t);
void vial_keycode_tap(uint16_t);
void test_vial_reload(void);
