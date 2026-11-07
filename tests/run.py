#!/usr/bin/env python3
"""Compile real keymaps and test the actual pinned action_tapping.c on the host.

Only hardware/report functions are mocked. This is not an RP2040 firmware build.
Usage: python3 tests/run.py /path/to/keebart-firmware
"""
import json
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

root = Path(__file__).resolve().parents[1]
firmware = Path(sys.argv[1]).resolve()
keymap = root / 'keyboards/keebart/corne_mx_pro/keymaps/basch_de'
cc = os.environ.get('CC', 'cc')
with tempfile.TemporaryDirectory(prefix='corne-host-') as directory:
    tmp = Path(directory)
    shutil.copy(root / 'tests/mock.h', tmp / 'mock.h')
    for name in ('action.h','action_layer.h','action_util.h','timer.h','via.h',
                 'quantum.h','wait.h','keymap_introspection.h','dynamic_keymap.h'):
        (tmp / name).write_text('#include "mock.h"\n')
    # Copy unchanged upstream engine and header so quoted hardware includes
    # resolve to the mock boundary. No source transformations are performed.
    for name in ('action_tapping.c','action_tapping.h'):
        shutil.copy(firmware / 'quantum' / name, tmp / name)
    for name in ('process_tap_dance.c','process_tap_dance.h'):
        shutil.copy(firmware/'quantum/process_keycode'/name, tmp/name)
    shutil.copy(root/'tests/vial.h', tmp/'vial.h')
    layout = json.loads((firmware / 'keyboards/keebart/corne_mx_pro/info.json').read_text())['layouts']['LAYOUT_split_3x6_3_ex2']['layout']
    matrix = [['KC_NO']*7 for _ in range(8)]
    for i, item in enumerate(layout):
        row, col = item['matrix']
        matrix[row][col] = f'k{i}'
    macro = '#define LAYOUT_split_3x6_3_ex2(' + ','.join(f'k{i}' for i in range(46)) + ') { '
    macro += ', '.join('{' + ','.join(row) + '}' for row in matrix) + ' }\n'
    (tmp / 'layout.h').write_text(macro)
    options = [cc, '-std=gnu11', '-Wall', '-Wextra', '-Wno-unused-parameter', '-Werror',
               '-DREPEAT_KEY_ENABLE', '-DENCODER_MAP_ENABLE', '-DNO_ACTION_ONESHOT', '-DTAP_DANCE_ENABLE',
               '-DQMK_KEYBOARD_H="mock.h"',
               '-I'+str(tmp), '-I'+str(keymap), '-I'+str(firmware/'quantum'),
               '-I'+str(firmware/'quantum/keymap_extras'), '-I'+str(firmware/'quantum/sequencer')]
    for name in ('basch_de','keebart_vial_stock'):
        path = keymap.parent / name / 'keymap.c'
        subprocess.run(options + ['-fsyntax-only', str(path)], check=True)
    shutil.copy(root/'tests/qmk_settings.h', tmp/'qmk_settings.h')
    # Link the real Vial timing and QMK Settings hooks without symbol renaming.
    vial = (firmware/'quantum/vial.c').read_text()
    start = vial.index('#ifdef VIAL_TAP_DANCE_ENABLE\n#include "process_tap_dance.h"')
    end = vial.index('#ifdef VIAL_COMBO_ENABLE', start)
    boundary = '''#include "mock.h"
#include "qmk_settings.h"
#include "vial.h"
'''
    (tmp/'vial-timing.c').write_text(boundary + vial[start:end] + '\nvoid test_vial_reload(void) { reload_tap_dance(); }\n')
    subprocess.run(options + ['-DQMK_SETTINGS', '-DTAPPING_TERM_PER_KEY', '-c',
                             str(tmp/'vial-timing.c'), '-o', str(tmp/'vial-timing.o')], check=True)
    settings = (firmware/'quantum/qmk_settings.c').read_text()
    hooks = settings[settings.index('uint16_t qs_get_tapping_term'):settings.index('uint16_t get_combo_term')]
    hooks += settings[settings.index('bool get_chordal_hold'):settings.index('uint16_t get_flow_tap_term')]
    (tmp/'settings-hooks.c').write_text('#include "mock.h"\n#include "qmk_settings.h"\n' + hooks)
    caps = (firmware/'quantum/process_keycode/process_caps_word.c').read_text()
    caps_default = caps[caps.index('__attribute__((weak)) bool caps_word_press_user'):]
    (tmp/'caps-default.c').write_text('#include "mock.h"\n' + caps_default)
    options += ['-DTAPPING_TERM_PER_KEY', '-DQUICK_TAP_TERM_PER_KEY', '-DPERMISSIVE_HOLD_PER_KEY', '-DHOLD_ON_OTHER_KEY_PRESS_PER_KEY', '-DCHORDAL_HOLD']
    subprocess.run(options + [str(tmp/'caps-default.c'), str(tmp/'settings-hooks.c'), str(tmp/'action_tapping.c'), str(tmp/'process_tap_dance.c'), str(keymap/'behaviors.c'), str(tmp/'vial-timing.o'),
                             str(root/'tests/sequences.c'), '-o', str(tmp/'sequences')], check=True)
    scenarios = ['tap','timeout','nested-opposite','nested-same','roll-opposite','same-timeout',
                 'thumb-left','thumb-right','thumb-hold','quick-hrm-in','quick-hrm-out',
                 'quick-tab-in','quick-tab-out','quick-enter-in','layer-roll','layer-nested',
                 'spotlight','caps-word','comma-dot','timer-wrap','defaults-preserve','defaults-new-image',
                 'defaults-slave','defaults-failure','defaults-dance-failure','gui-timing',
                 'td-hyper-hold','td-short-hold','td-comma-tap','td-interrupt','td-double','td-gui-edit']
    for scenario in scenarios:
        subprocess.run([str(tmp/'sequences'), scenario], check=True)
    print(f'{len(scenarios)} host regression scenarios and compiled keymap checks passed; both keymaps compile with real QMK keycodes/layout.')
