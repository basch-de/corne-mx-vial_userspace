// SPDX-License-Identifier: GPL-2.0-or-later
#include "mock.h"
#include "layers.h"
#include "qmk_settings.h"
#include "vial.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "../keyboards/keebart/corne_mx_pro/keymaps/basch_de/keymap.c"

_Static_assert(sizeof(keymaps) / sizeof(keymaps[0]) == 8, "Expected eight default layers");
_Static_assert(sizeof(encoder_map) / sizeof(encoder_map[0]) == 8, "Encoder layers must match");

static void check_keymap(void) {
    assert(MACOS == 0 && NUM == 1 && SYM == 2 && NAV == 3 && WIN == 4 &&
           SHORT == 5 && NUMPAD == 6 && SYS == 7);
    assert(keymaps[MACOS][0][0] == LT(SYS, KC_GRV));
    assert(keymaps[MACOS][4][0] == KC_BSLS);
    assert(keymaps[MACOS][1][1] == LCTL_T(KC_A));
    assert(keymaps[MACOS][3][3] == LT(NAV, KC_ESC));
    assert(keymaps[MACOS][3][4] == LT(NUM, KC_BSPC));
    assert(keymaps[MACOS][3][5] == LT(WIN, KC_TAB));
    assert(keymaps[MACOS][7][5] == LT(NUMPAD, KC_ENT));
    assert(keymaps[MACOS][7][4] == LT(SYM, KC_SPC));
    assert(keymaps[MACOS][2][0] == LT(SHORT, KC_EQL));
    assert(keymaps[SYM][2][5] == TG(NUMPAD) && keymaps[NUMPAD][2][5] == TG(NUMPAD));
    // Four physical inner keys are still unassigned. Update when assigning them.
    for (unsigned layer = MACOS; layer <= SYS; ++layer) {
        assert(keymaps[layer][0][6] == KC_NO && keymaps[layer][1][6] == KC_NO);
        assert(keymaps[layer][4][6] == KC_NO && keymaps[layer][5][6] == KC_NO);
        if (layer != SYS) {
            for (unsigned row = 0; row < MATRIX_ROWS; ++row) {
                for (unsigned col = 0; col < MATRIX_COLS; ++col) {
                    uint16_t code = keymaps[layer][row][col];
                    assert(code != QK_RBT && code != QK_BOOT && code != EE_CLR);
                }
            }
        }
    }
    for (unsigned col = 3; col <= 5; ++col) {
        assert(chordal_hold_layout[3][col] == '*' && chordal_hold_layout[7][col] == '*');
    }
    // Check the actual compiled matrix, including the mirrored right half.
    for (unsigned row = 2; row <= 6; row += 4) {
        assert(keymaps[SYS][row][0] == QK_RBT);
        assert(keymaps[SYS][row][1] == QK_BOOT);
        assert(keymaps[SYS][row][2] == EE_CLR);
    }
    assert(keymaps[MACOS][1][0] == TD(0));
    assert(keymaps[MACOS][7][3] == TD(1));
    assert(keymaps[NUMPAD][6][1] == TD(2));
    assert(keymaps[SHORT][6][4] == KC_ASST);
    assert(keymaps[SYS][4][0] == KC_PWR);
}

static uint16_t now, registered16, registered8, consumer, weak_mods;
static uint8_t input_layer = MACOS;
static unsigned caps_toggles, reports, overflow;
layer_state_t layer_state;
static struct { keypos_t key; uint16_t code; bool pressed; uint8_t taps; } trace[128];
static unsigned trace_count, input_count;
static uint32_t user_eeprom;
static unsigned settings_writes, dance_writes;
static int fail_dance_index = -1;
static vial_tap_dance_entry_t stored_dances[VIAL_TAP_DANCE_ENTRIES];
static struct { uint16_t code; bool pressed; } outputs[64];
static unsigned output_count;
int dynamic_keymap_get_tap_dance(uint8_t index, vial_tap_dance_entry_t *entry) {
    if (index >= VIAL_TAP_DANCE_ENTRIES) return -1;
    *entry = stored_dances[index]; return 0;
}
int dynamic_keymap_set_tap_dance(uint8_t index, const vial_tap_dance_entry_t *entry) {
    if (index >= VIAL_TAP_DANCE_ENTRIES || index == fail_dance_index) return -1;
    stored_dances[index] = *entry; dance_writes++; return 0;
}
void vial_keycode_down(uint16_t code) {
    assert(output_count < 64);
    outputs[output_count++] = (__typeof__(outputs[0])){code,true};
    if (IS_QK_MOMENTARY(code)) layer_state |= 1UL << QK_MOMENTARY_GET_LAYER(code);
    else if (code == CW_TOGG) caps_word_toggle();
    else if (code == KC_DOT) register_code(code);
    else register_code16(code);
}
void vial_keycode_up(uint16_t code) {
    assert(output_count < 64);
    outputs[output_count++] = (__typeof__(outputs[0])){code,false};
    if (IS_QK_MOMENTARY(code)) layer_state &= ~(1UL << QK_MOMENTARY_GET_LAYER(code));
    else if (code == CW_TOGG) {}
    else if (code == KC_DOT) unregister_code(code);
    else unregister_code16(code);
}
void vial_keycode_tap(uint16_t code) { vial_keycode_down(code); vial_keycode_up(code); }

static bool via_valid = true, master = true;
static int fail_setting = -1;
test_settings_t QS = {.tapping_term=TAPPING_TERM, .quick_tap_term=TAPPING_TERM};
bool via_eeprom_is_valid(void) { return via_valid; }
bool is_keyboard_master(void) { return master; }
uint32_t eeconfig_read_user(void) { return user_eeprom; }
void eeconfig_update_user(uint32_t value) { user_eeprom = value; }
int qmk_settings_set(uint16_t id, const void *value, size_t size) {
    if (id == fail_setting) return -1;
    settings_writes++;
    if (id == 7) { assert(size == 2); memcpy(&QS.tapping_term, value, size); }
    else {
        assert(size == 1);
        uint8_t bit = id == 22 ? 1 : id == 23 ? 2 : id == 26 ? 8 : 0;
        assert(bit);
        if (*(const uint8_t *)value) QS.tapping_v2 |= bit;
        else QS.tapping_v2 &= ~bit;
    }
    return 0;
}

uint16_t get_record_keycode(keyrecord_t *r, bool cache) {
    return r->keycode;
}
bool is_tap_record(keyrecord_t *r) {
    uint16_t code = get_record_keycode(r, false);
    return IS_EVENT(r->event) && (IS_QK_MOD_TAP(code) || IS_QK_LAYER_TAP(code));
}
void process_record(keyrecord_t *r) {
    if (IS_NOEVENT(r->event)) return;
    uint16_t code = get_record_keycode(r, false);
    preprocess_tap_dance(code, r);
    assert(trace_count < 128);
    trace[trace_count++] = (__typeof__(trace[0])){r->event.key, code, r->event.pressed, r->tap.count};
    process_tap_dance(code, r);
    if (IS_QK_LAYER_TAP(code) && !r->tap.count) {
        uint32_t bit = 1UL << QK_LAYER_TAP_GET_LAYER(code);
        layer_state = r->event.pressed ? layer_state | bit : layer_state & ~bit;
    }
}
void process_record_tap_hint(keyrecord_t *r) {}
void clear_keyboard(void) { overflow++; }
action_t layer_switch_get_action(keypos_t key) { return (action_t){.code = 0}; }
uint16_t timer_read(void) { return now; }
uint16_t timer_elapsed(uint16_t start) { return (uint16_t)(now - start); }
void host_consumer_send(uint16_t code) { consumer = code; reports++; }
void register_code16(uint16_t code) { registered16 = code; }
void unregister_code16(uint16_t code) { assert(registered16 == code); registered16 = 0; }
void register_code(uint8_t code) { registered8 = code; }
void unregister_code(uint8_t code) { assert(registered8 == code); registered8 = 0; }
void caps_word_toggle(void) { caps_toggles++; }
void add_weak_mods(uint8_t mods) { weak_mods |= mods; }
uint8_t get_mods(void) { return 0; }
uint8_t get_weak_mods(void) { return weak_mods; }
void del_weak_mods(uint8_t mods) { weak_mods &= ~mods; }
void clear_weak_mods(void) { weak_mods = 0; }
void send_keyboard_report(void) {}
void layer_move(uint8_t layer) { layer_state = 1UL << layer; }
void layer_invert(uint8_t layer) { layer_state ^= 1UL << layer; }
void wait_ms(uint16_t ms) {}
void qs_wait_ms(uint16_t ms) {}


// Use compiled bindings from the scenario's layer; layer lookup and reports are mocked.
static void event(uint8_t row, uint8_t col, uint16_t time, bool pressed) {
    now = time;
    input_count++;
    action_tapping_process((keyrecord_t){.event = {.key={.row=row,.col=col}, .time=time, .pressed=pressed, .type=KEY_EVENT}, .keycode=keymaps[input_layer][row][col]});
}
static void tick(uint16_t time) {
    now = time;
    tap_dance_task();
    action_tapping_process((keyrecord_t){.event={.time=time, .type=TICK_EVENT}});
}
static unsigned press_taps(uint16_t code, unsigned index) {
    for (unsigned i=0; i<trace_count; i++) {
        if (trace[i].code == code && trace[i].pressed && index-- == 0) return trace[i].taps;
    }
    fprintf(stderr, "Missing press: 0x%x\n", code);
    abort();
}
static void hrm(bool pressed, uint16_t time) { event(1,1,time,pressed); }

// Adapted from Studio's checks: flush buffers and require balanced physical events.
static void check_released(void) {
    tick((uint16_t)(now + 1000));
    bool down[MATRIX_ROWS][MATRIX_COLS] = {0};
    uint8_t taps[MATRIX_ROWS][MATRIX_COLS] = {0};
    for (unsigned i = 0; i < trace_count; ++i) {
        keypos_t key = trace[i].key;
        assert(down[key.row][key.col] != trace[i].pressed);
        if (trace[i].pressed) taps[key.row][key.col] = trace[i].taps;
        else assert(taps[key.row][key.col] == trace[i].taps);
        down[key.row][key.col] = trace[i].pressed;
    }
    for (unsigned row = 0; row < MATRIX_ROWS; ++row) {
        for (unsigned col = 0; col < MATRIX_COLS; ++col) assert(!down[row][col]);
    }
    // Tap Dance's emitted codes must also have matching press/release pairs.
    for (unsigned i = 0; i < output_count; ++i) {
        int balance = 0;
        for (unsigned j = 0; j < output_count; ++j) {
            if (outputs[j].code != outputs[i].code) continue;
            balance += outputs[j].pressed ? 1 : -1;
            assert(balance >= 0 && balance <= 1);
        }
        assert(balance == 0);
    }
    assert(trace_count == input_count && overflow == 0 && layer_state == 0);
    assert(registered16 == 0 && registered8 == 0);
}

int main(int argc, char **argv) {
    assert(argc == 2);
    check_keymap();
    const char *test = argv[1];
    if (!strcmp(test,"comma-dot") || !strcmp(test,"td-comma-tap") || !strcmp(test,"td-double")) input_layer = NUMPAD;
    if (!strcmp(test,"defaults-slave")) master = false;
    if (!strcmp(test,"defaults-old-marker")) user_eeprom = 0x42535401UL;
    stored_dances[3].on_tap = KC_Z;
    via_init_kb();
    keyboard_post_init_user();
    test_vial_reload();
    if (master) {
        assert(user_eeprom != 0 && dance_writes == 3);
        assert(QS.tapping_term == 250 && QS.tapping_v2 == 9 && settings_writes == 4);
    }
    assert(stored_dances[3].on_tap == KC_Z);
    if (!strcmp(test,"tap")) {
        hrm(true,100); hrm(false,150);
        assert(press_taps(LCTL_T(KC_A),0) == 1);
    } else if (!strcmp(test,"timeout")) {
        hrm(true,100); tick(381); hrm(false,400);
        assert(press_taps(LCTL_T(KC_A),0) == 0);
    } else if (!strcmp(test,"nested-opposite") || !strcmp(test,"nested-same")) {
        bool opposite = !strcmp(test,"nested-opposite");
        uint8_t row = opposite ? 4 : 0;
        hrm(true,100); event(row,2,150,true); event(row,2,180,false); hrm(false,220);
        assert(press_taps(LCTL_T(KC_A),0) == (opposite ? 0 : 1));
    } else if (!strcmp(test,"roll-opposite")) {
        hrm(true,100); event(4,5,130,true); hrm(false,160); event(4,5,190,false);
        assert(press_taps(LCTL_T(KC_A),0) == 1);
    } else if (!strcmp(test,"same-timeout")) {
        hrm(true,100); event(0,1,150,true); tick(381); hrm(false,420); event(0,1,450,false);
        assert(press_taps(LCTL_T(KC_A),0) == 1);
    } else if (!strcmp(test,"thumb-left") || !strcmp(test,"thumb-right")) {
        uint8_t row = !strcmp(test,"thumb-left") ? 3 : 7;
        hrm(true,100); event(row,4,130,true); event(row,4,170,false); hrm(false,200);
        assert(press_taps(LCTL_T(KC_A),0) == 0);
    } else if (!strcmp(test,"quick-hrm-in") || !strcmp(test,"quick-hrm-out")) {
        bool inside = !strcmp(test,"quick-hrm-in");
        hrm(true,100); hrm(false,180); hrm(true,inside ? 400 : 440); tick(700); hrm(false,720);
        assert(press_taps(LCTL_T(KC_A),1) == (inside ? 2 : 0));
    } else if (!strcmp(test,"quick-tab-in") || !strcmp(test,"quick-tab-out") || !strcmp(test,"quick-enter-in")) {
        bool inside = strcmp(test,"quick-tab-out") != 0;
        uint16_t code = !strcmp(test,"quick-enter-in") ? LT(NUMPAD, KC_ENT) : LT(WIN, KC_TAB);
        uint8_t row = !strcmp(test,"quick-enter-in") ? 7 : 3;
        event(row,5,100,true); event(row,5,120,false);
        event(row,5,inside ? 340 : 380,true); tick(700);
        assert(layer_state == (inside ? 0 : (1UL << (row == 7 ? NUMPAD : WIN))));
        event(row,5,720,false);
        assert(press_taps(code,1) == (inside ? 2 : 0));
    } else if (!strcmp(test,"layer-roll") || !strcmp(test,"layer-nested") || !strcmp(test,"thumb-hold")) {
        bool nested = strcmp(test,"layer-roll") != 0;
        bool tab = !strcmp(test,"thumb-hold");
        uint16_t code = tab ? LT(WIN, KC_TAB) : LT(SYM, KC_SPC);
        uint8_t row = tab ? 3 : 7, col = tab ? 5 : 4;
        event(row,col,100,true); event(0,1,130,true);
        if (nested) {
            event(0,1,160,false);
            assert(layer_state == (1UL << (tab ? WIN : SYM)));
            event(row,col,180,false);
        } else { event(row,col,160,false); event(0,1,180,false); }
        assert(press_taps(code,0) == (nested ? 0 : 1));
    } else if (!strcmp(test,"spotlight")) {
        event(1,0,100,true); event(1,0,150,false);
        assert(output_count == 0); tick(351);
        assert(output_count == 2 && outputs[0].code == LGUI(KC_SPC) && outputs[0].pressed);
        assert(!outputs[1].pressed && registered16 == 0);
    } else if (!strcmp(test,"caps-word")) {
        assert(get_tapping_term(TD(1), &(keyrecord_t){0}) == 250);
        event(7,3,100,true); event(7,3,150,false); tick(351);
        assert(caps_toggles == 1 && output_count == 2 && outputs[0].code == CW_TOGG);
        assert(CAPS_WORD_IDLE_TIMEOUT == 5000);
        assert(caps_word_press_user(KC_MINS) && weak_mods == MOD_BIT(KC_LSFT));
        clear_weak_mods();
        assert(caps_word_press_user(KC_A) && weak_mods == MOD_BIT(KC_LSFT));
        assert(!caps_word_press_user(KC_SPC));
    } else if (!strcmp(test,"comma-dot")) {
        event(6,1,100,true); tick(351);
        assert(registered8 == KC_DOT && layer_state == 0);
        event(6,1,400,false); assert(registered8 == 0);
    } else if (!strcmp(test,"td-hyper-hold")) {
        event(1,0,100,true); tick(351);
        assert(registered16 == HYPR(KC_NO) && outputs[0].code == HYPR(KC_NO));
        event(1,0,400,false); assert(registered16 == 0);
    } else if (!strcmp(test,"td-short-hold")) {
        event(7,3,100,true); tick(351);
        assert(layer_state_is(SHORT) && caps_toggles == 0);
        event(7,3,400,false); assert(!layer_state_is(SHORT));
    } else if (!strcmp(test,"td-comma-tap")) {
        event(6,1,100,true); event(6,1,150,false); tick(351);
        assert(output_count == 2 && outputs[0].code == KC_COMM && !outputs[1].pressed);
    } else if (!strcmp(test,"td-interrupt")) {
        event(7,3,100,true); event(0,1,130,true);
        assert(caps_toggles == 1 && !layer_state_is(SHORT));
        event(7,3,150,false); event(0,1,160,false);
    } else if (!strcmp(test,"td-double")) {
        event(6,1,100,true); event(6,1,150,false);
        event(6,1,180,true); event(6,1,210,false); tick(431);
        assert(output_count == 4);
        for (unsigned i=0; i<4; ++i) assert(outputs[i].code == KC_COMM);
    } else if (!strcmp(test,"td-gui-edit")) {
        vial_tap_dance_entry_t entry = {KC_X, KC_Y, KC_Z, KC_NO, 400};
        dynamic_keymap_set_tap_dance(0, &entry);
        assert(get_tapping_term(TD(0), &(keyrecord_t){0}) == 400);
        via_init_kb(); keyboard_post_init_user();
        event(1,0,100,true); event(1,0,150,false);
        tick(351); assert(output_count == 0);
        tick(501); assert(output_count == 2 && outputs[0].code == KC_X);
    } else if (!strcmp(test,"timer-wrap")) {
        hrm(true,65500); hrm(false,20); hrm(true,100); tick(500); hrm(false,550);
        assert(press_taps(LCTL_T(KC_A),1) == 2);
    } else if (!strcmp(test,"defaults-preserve")) {
        QS.tapping_term = 315; QS.tapping_v2 = 2;
        stored_dances[0].on_tap = KC_Z;
        via_init_kb(); keyboard_post_init_user();
        assert(stored_dances[0].on_tap == KC_Z && dance_writes == 3);
        assert(QS.tapping_term == 315 && QS.tapping_v2 == 2 && settings_writes == 4);
    } else if (!strcmp(test,"defaults-new-image")) {
        QS.tapping_term = 315; QS.tapping_v2 = 2; via_valid = false;
        via_init_kb(); keyboard_post_init_user();
        assert(QS.tapping_term == 250 && QS.tapping_v2 == 9 && settings_writes == 8 && dance_writes == 6);
    } else if (!strcmp(test,"defaults-slave")) {
        assert(user_eeprom == 0 && settings_writes == 0 && dance_writes == 0);
        master = true; keyboard_post_init_user();
        assert(user_eeprom != 0 && settings_writes == 4 && dance_writes == 3);
    } else if (!strcmp(test,"defaults-failure")) {
        user_eeprom = 0; fail_setting = 23; keyboard_post_init_user();
        assert(user_eeprom == 0 && settings_writes == 6 && dance_writes == 3);
        fail_setting = -1; keyboard_post_init_user();
        assert(user_eeprom != 0 && settings_writes == 10 && dance_writes == 6);
    } else if (!strcmp(test,"defaults-dance-failure")) {
        user_eeprom = 0; fail_dance_index = 1; keyboard_post_init_user();
        assert(user_eeprom == 0);
        fail_dance_index = -1; keyboard_post_init_user(); assert(user_eeprom != 0);
    } else if (!strcmp(test,"defaults-old-marker")) {
        assert(user_eeprom != 0x42535401UL);
    } else if (!strcmp(test,"defaults-qmk-init")) {
        eeconfig_init_user();
        assert(user_eeprom == 0 && settings_writes == 4 && dance_writes == 3);
        via_init_kb(); keyboard_post_init_user();
        assert(user_eeprom != 0 && settings_writes == 8 && dance_writes == 6);
    } else if (!strcmp(test,"defaults-unrelated")) {
        QS.quick_tap_term = 315;
        stored_dances[3].custom_tapping_term = 410;
        user_eeprom = 0; keyboard_post_init_user();
        assert(QS.quick_tap_term == 315 && stored_dances[3].on_tap == KC_Z);
        assert(stored_dances[3].custom_tapping_term == 410);
    } else if (!strcmp(test,"gui-timing")) {
        uint16_t term = 350; qmk_settings_set(7, &term, sizeof(term));
        hrm(true,100); tick(400); assert(trace_count == 0);
        tick(451); hrm(false,500); assert(press_taps(LCTL_T(KC_A),0) == 0);
    } else { abort(); }
    check_released();
    printf("PASS %s\n", test);
}
