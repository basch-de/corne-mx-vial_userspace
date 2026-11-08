// SPDX-License-Identifier: GPL-2.0-or-later
#include QMK_KEYBOARD_H
#include "layers.h"

#include "qmk_settings.h"
#include "via.h"
#include "vial.h"
#include "dynamic_keymap.h"

// Reapply defaults only for uninitialized user state or a new Vial image.
// Ordinary restarts must preserve changes made through the configurator.
#define USER_SETTINGS_VERSION 0x42530102UL
static bool vial_settings_invalid;

// Standard Vial Tap Dance slots: tap, hold, double tap, tap-then-hold, timeout.
// Empty double actions use Vial's built-in repeat/fallback behavior.
static const vial_tap_dance_entry_t default_tap_dances[] = {
    {LGUI(KC_SPC), HYPR(KC_NO), KC_NO, KC_NO, TAPPING_TERM},
    {CW_TOGG, MO(SHORT), KC_NO, KC_NO, TAPPING_TERM},
    {KC_COMM, KC_DOT, KC_NO, KC_NO, TAPPING_TERM},
};

void via_init_kb(void) {
    vial_settings_invalid = !via_eeprom_is_valid();
}

void eeconfig_init_user(void) {
    eeconfig_update_user(0);
}

void keyboard_post_init_user(void) {
    if (!is_keyboard_master()) return;
    if (!vial_settings_invalid && eeconfig_read_user() == USER_SETTINGS_VERSION) return;

    const uint16_t term = TAPPING_TERM;
    const uint8_t enabled = 1;
    const uint8_t disabled = 0;
    // Protocol IDs checked against qmk_settings.c by scripts/check_sources.py.
    if (qmk_settings_set(7, &term, sizeof(term)) == 0 &&
        qmk_settings_set(22, &enabled, sizeof(enabled)) == 0 &&
        qmk_settings_set(23, &disabled, sizeof(disabled)) == 0 &&
        qmk_settings_set(26, &enabled, sizeof(enabled)) == 0) {
        for (uint8_t i = 0; i < sizeof(default_tap_dances) / sizeof(default_tap_dances[0]); ++i) {
            vial_tap_dance_entry_t entry = default_tap_dances[i];
            if (dynamic_keymap_set_tap_dance(i, &entry) != 0) return;
        }
        eeconfig_update_user(USER_SETTINGS_VERSION);
        vial_settings_invalid = false;
    }
}
