// Host-only QMK boundary. Tapping decisions execute the real upstream engine.
#ifndef TEST_QMK_MOCK_H
#define TEST_QMK_MOCK_H
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "keycode.h"
#include "quantum_keycodes.h"
#include "action_code.h"
#include "config.h"
#include "caps_word.h"

#define MATRIX_ROWS 8
#define MATRIX_COLS 7
#define NUM_ENCODERS 4
#define NUM_DIRECTIONS 2
#define TAP_CODE_DELAY 0
#define ENCODER_CCW_CW(a,b) {b,a}
#define PROGMEM
#define pgm_read_byte(p) (*(const char *)(p))
#define ac_dprintf(...) ((void)0)
#define debug_event(...) ((void)0)
#define debug_record(...) ((void)0)
#define KEY_EVENT 1
#define TICK_EVENT 0
typedef struct { uint8_t col, row; } keypos_t;
typedef struct { keypos_t key; uint16_t time; bool pressed; uint8_t type; } keyevent_t;
typedef struct { bool interrupted:1; uint8_t reserved:3; uint8_t count:4; } tap_t;
typedef struct { keyevent_t event; tap_t tap; uint16_t keycode; } keyrecord_t;
typedef uint32_t layer_state_t;
extern layer_state_t layer_state;
#define layer_state_is(n) (!!(layer_state & (1UL << (n))))
#define KEYEQ(a,b) ((a).row == (b).row && (a).col == (b).col)
#define IS_NOEVENT(e) ((e).type == TICK_EVENT)
#define IS_EVENT(e) (!IS_NOEVENT(e))
#define TIMER_DIFF_16(a,b) ((uint16_t)((a)-(b)))

uint16_t get_record_keycode(keyrecord_t *, bool);
bool is_tap_record(keyrecord_t *);
void process_record(keyrecord_t *);
void process_record_tap_hint(keyrecord_t *);
void clear_keyboard(void);
action_t layer_switch_get_action(keypos_t);
uint16_t timer_read(void);
uint16_t timer_elapsed(uint16_t);
void host_consumer_send(uint16_t);
void register_code16(uint16_t);
void unregister_code16(uint16_t);
void register_code(uint8_t);
void unregister_code(uint8_t);
void caps_word_toggle(void);
void add_weak_mods(uint8_t);
uint8_t get_mods(void);
uint8_t get_weak_mods(void);
void del_weak_mods(uint8_t);
void clear_weak_mods(void);
void send_keyboard_report(void);
void layer_move(uint8_t);
void layer_invert(uint8_t);
void wait_ms(uint16_t);
void qs_wait_ms(uint16_t);
bool via_eeprom_is_valid(void);
bool is_keyboard_master(void);
uint32_t eeconfig_read_user(void);
void eeconfig_update_user(uint32_t);
void via_init_kb(void);
void eeconfig_init_user(void);
void keyboard_post_init_user(void);
bool caps_word_press_user(uint16_t);
#include "action_tapping.h"
#include "layout.h"
bool preprocess_tap_dance(uint16_t, keyrecord_t *);
bool process_tap_dance(uint16_t, keyrecord_t *);
void tap_dance_task(void);
uint16_t tap_dance_count(void);
struct tap_dance_action_t;
struct tap_dance_action_t *tap_dance_get(uint16_t);
#endif
