// SPDX-License-Identifier: GPL-2.0-or-later
#include "mock.h"
#include "behaviors.h"
#include "qmk_settings.h"
#include "vial.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static uint16_t codes[MATRIX_ROWS][MATRIX_COLS];
static uint16_t now, registered16, registered8, consumer, weak_mods;
static unsigned caps_toggles, reports, overflow;
layer_state_t layer_state;
static struct { uint16_t code; bool pressed; uint8_t taps; } trace[128];
static unsigned trace_count;
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

static bool via_valid = true, master = true, fail_write;
test_settings_t QS = {.tapping_term=TAPPING_TERM, .quick_tap_term=TAPPING_TERM};
bool via_eeprom_is_valid(void) { return via_valid; }
bool is_keyboard_master(void) { return master; }
uint32_t eeconfig_read_user(void) { return user_eeprom; }
void eeconfig_update_user(uint32_t value) { user_eeprom = value; }
int qmk_settings_set(uint16_t id, const void *value, size_t size) {
    if (fail_write) return -1;
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
    return r->keycode ? r->keycode : codes[r->event.key.row][r->event.key.col];
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
    trace[trace_count++] = (__typeof__(trace[0])){code, r->event.pressed, r->tap.count};
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


static void event(uint16_t code, uint8_t row, uint8_t col, uint16_t time, bool pressed) {
    now = time;
    codes[row][col] = code;
    action_tapping_process((keyrecord_t){.event = {.key={.row=row,.col=col}, .time=time, .pressed=pressed, .type=KEY_EVENT}, .keycode=code});
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
static void hrm(bool pressed, uint16_t time) { event(LCTL_T(KC_A),1,1,time,pressed); }

int main(int argc, char **argv) {
    assert(argc == 2);
    const char *test = argv[1];
    via_init_kb();
    keyboard_post_init_user();
    test_vial_reload();
    assert(dance_writes == 3);
    assert(QS.tapping_term == 250 && QS.tapping_v2 == 9 && settings_writes == 4);
    if (!strcmp(test,"tap")) {
        hrm(true,100); hrm(false,150);
        assert(press_taps(LCTL_T(KC_A),0) == 1);
    } else if (!strcmp(test,"timeout")) {
        hrm(true,100); tick(381); hrm(false,400);
        assert(press_taps(LCTL_T(KC_A),0) == 0);
    } else if (!strcmp(test,"nested-opposite") || !strcmp(test,"nested-same")) {
        bool opposite = !strcmp(test,"nested-opposite");
        uint8_t row = opposite ? 4 : 0;
        hrm(true,100); event(KC_Q,row,2,150,true); event(KC_Q,row,2,180,false); hrm(false,220);
        assert(press_taps(LCTL_T(KC_A),0) == (opposite ? 0 : 1));
    } else if (!strcmp(test,"roll-opposite")) {
        hrm(true,100); event(KC_Y,4,5,130,true); hrm(false,160); event(KC_Y,4,5,190,false);
        assert(press_taps(LCTL_T(KC_A),0) == 1);
    } else if (!strcmp(test,"same-timeout")) {
        hrm(true,100); event(KC_Q,0,1,150,true); tick(381); hrm(false,420); event(KC_Q,0,1,450,false);
        assert(press_taps(LCTL_T(KC_A),0) == 1);
    } else if (!strcmp(test,"thumb-left") || !strcmp(test,"thumb-right")) {
        uint8_t row = !strcmp(test,"thumb-left") ? 3 : 7;
        hrm(true,100); event(KC_ENT,row,4,130,true); event(KC_ENT,row,4,170,false); hrm(false,200);
        assert(press_taps(LCTL_T(KC_A),0) == 0);
    } else if (!strcmp(test,"quick-hrm-in") || !strcmp(test,"quick-hrm-out")) {
        bool inside = !strcmp(test,"quick-hrm-in");
        hrm(true,100); hrm(false,180); hrm(true,inside ? 400 : 440); tick(700); hrm(false,720);
        assert(press_taps(LCTL_T(KC_A),1) == (inside ? 2 : 0));
    } else if (!strcmp(test,"quick-tab-in") || !strcmp(test,"quick-tab-out") || !strcmp(test,"quick-enter-in")) {
        bool inside = strcmp(test,"quick-tab-out") != 0;
        uint16_t code = !strcmp(test,"quick-enter-in") ? ENT_PAD : TAB_WIN;
        event(code,3,5,100,true); event(code,3,5,120,false);
        event(code,3,5,inside ? 340 : 380,true); tick(700); event(code,3,5,720,false);
        assert(press_taps(code,1) == (inside ? 2 : 0));
    } else if (!strcmp(test,"layer-roll") || !strcmp(test,"layer-nested") || !strcmp(test,"thumb-hold")) {
        bool nested = strcmp(test,"layer-roll") != 0;
        uint16_t code = !strcmp(test,"thumb-hold") ? TAB_WIN : SPC_SYM;
        event(code,7,4,100,true); event(KC_Q,0,1,130,true);
        if (nested) { event(KC_Q,0,1,160,false); event(code,7,4,180,false); }
        else { event(code,7,4,160,false); event(KC_Q,0,1,180,false); }
        assert(press_taps(code,0) == (nested ? 0 : 1));
    } else if (!strcmp(test,"spotlight")) {
        event(TD(0),1,0,100,true); event(TD(0),1,0,150,false);
        assert(output_count == 0); tick(351);
        assert(output_count == 2 && outputs[0].code == LGUI(KC_SPC) && outputs[0].pressed);
        assert(!outputs[1].pressed && registered16 == 0);
    } else if (!strcmp(test,"caps-word")) {
        assert(get_tapping_term(TD(1), &(keyrecord_t){0}) == 250);
        event(TD(1),7,3,100,true); event(TD(1),7,3,150,false); tick(351);
        assert(caps_toggles == 1 && output_count == 2 && outputs[0].code == CW_TOGG);
        assert(CAPS_WORD_IDLE_TIMEOUT == 5000);
        assert(caps_word_press_user(KC_MINS) && weak_mods == MOD_BIT(KC_LSFT));
        clear_weak_mods();
        assert(caps_word_press_user(KC_A) && weak_mods == MOD_BIT(KC_LSFT));
        assert(!caps_word_press_user(KC_SPC));
    } else if (!strcmp(test,"comma-dot")) {
        event(TD(2),6,1,100,true); tick(351);
        assert(registered8 == KC_DOT && layer_state == 0);
        event(TD(2),6,1,400,false); assert(registered8 == 0);
    } else if (!strcmp(test,"td-hyper-hold")) {
        event(TD(0),1,0,100,true); tick(351);
        assert(registered16 == HYPR(KC_NO) && outputs[0].code == HYPR(KC_NO));
        event(TD(0),1,0,400,false); assert(registered16 == 0);
    } else if (!strcmp(test,"td-short-hold")) {
        event(TD(1),7,3,100,true); tick(351);
        assert(layer_state_is(SHORT) && caps_toggles == 0);
        event(TD(1),7,3,400,false); assert(!layer_state_is(SHORT));
    } else if (!strcmp(test,"td-comma-tap")) {
        event(TD(2),6,1,100,true); event(TD(2),6,1,150,false); tick(351);
        assert(output_count == 2 && outputs[0].code == KC_COMM && !outputs[1].pressed);
    } else if (!strcmp(test,"td-interrupt")) {
        event(TD(1),7,3,100,true); event(KC_Q,0,1,130,true);
        assert(caps_toggles == 1 && !layer_state_is(SHORT));
        event(TD(1),7,3,150,false); event(KC_Q,0,1,160,false);
    } else if (!strcmp(test,"td-double")) {
        event(TD(2),6,1,100,true); event(TD(2),6,1,150,false);
        event(TD(2),6,1,180,true); event(TD(2),6,1,210,false); tick(431);
        assert(output_count == 4);
        for (unsigned i=0; i<4; ++i) assert(outputs[i].code == KC_COMM);
    } else if (!strcmp(test,"td-gui-edit")) {
        vial_tap_dance_entry_t entry = {KC_X, KC_Y, KC_Z, KC_NO, 400};
        dynamic_keymap_set_tap_dance(0, &entry);
        assert(get_tapping_term(TD(0), &(keyrecord_t){0}) == 400);
        via_init_kb(); keyboard_post_init_user();
        event(TD(0),1,0,100,true); event(TD(0),1,0,150,false);
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
        master = false; user_eeprom = 0; keyboard_post_init_user();
        assert(user_eeprom == 0 && settings_writes == 4);
    } else if (!strcmp(test,"defaults-failure")) {
        user_eeprom = 0; fail_write = true; keyboard_post_init_user();
        assert(user_eeprom == 0);
        fail_write = false; keyboard_post_init_user(); assert(user_eeprom != 0);
    } else if (!strcmp(test,"defaults-dance-failure")) {
        user_eeprom = 0; fail_dance_index = 1; keyboard_post_init_user();
        assert(user_eeprom == 0);
        fail_dance_index = -1; keyboard_post_init_user(); assert(user_eeprom != 0);
    } else if (!strcmp(test,"gui-timing")) {
        uint16_t term = 350; qmk_settings_set(7, &term, sizeof(term));
        hrm(true,100); tick(400); assert(trace_count == 0);
        tick(451); hrm(false,500); assert(press_taps(LCTL_T(KC_A),0) == 0);
    } else { abort(); }
    assert(overflow == 0);
    printf("PASS %s\n", test);
}
