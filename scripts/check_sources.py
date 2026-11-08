#!/usr/bin/env python3
"""Read-only compatibility checks against a manufacturer firmware checkout."""
import argparse
import json
import re
import subprocess
from pathlib import Path

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('firmware', type=Path)
args = parser.parse_args()

root = Path(__file__).resolve().parents[1]
firmware = args.firmware.resolve()
revision = subprocess.check_output(['git', '-C', str(firmware), 'rev-parse', 'HEAD'], text=True).strip()
print(f'Checking Keebart firmware commit: {revision}')
upstream = firmware / 'keyboards/keebart/corne_mx_pro'
local = root / 'keyboards/keebart/corne_mx_pro/keymaps'
assert 'SPLIT_HAND_PIN GP21' in (upstream / 'config.h').read_text()
assert (upstream / 'standard/keyboard.json').exists()
config = (upstream / 'config.h').read_text()
assert re.search(r'^#define\s+DYNAMIC_KEYMAP_LAYER_COUNT\s+16\s*$', config, re.M), 'Manufacturer 16-layer storage layout changed'
stock_config = (upstream / 'keymaps/vial/config.h').read_text()
personal_config = (local / 'basch_de/config.h').read_text()
for name in ('VIAL_KEYBOARD_UID', 'VIAL_UNLOCK_COMBO_ROWS', 'VIAL_UNLOCK_COMBO_COLS'):
    pattern = rf'^#define\s+{name}\s+(.+)$'
    stock = re.search(pattern, stock_config, re.M)
    personal = re.search(pattern, personal_config, re.M)
    assert stock and personal and re.sub(r'\s+', '', stock[1]) == re.sub(r'\s+', '', personal[1]), f'Manufacturer {name} changed'
layout = json.loads((upstream / 'info.json').read_text())['layouts']['LAYOUT_split_3x6_3_ex2']['layout']
# Independent electrical order, including all mirrored right-hand positions.
expected = ([[0,c] for c in range(7)] + [[4,c] for c in range(6,-1,-1)] +
            [[1,c] for c in range(7)] + [[5,c] for c in range(6,-1,-1)] +
            [[2,c] for c in range(6)] + [[6,c] for c in range(5,-1,-1)] +
            [[3,c] for c in range(3,6)] + [[7,c] for c in range(5,2,-1)])
assert [x['matrix'] for x in layout] == expected, '46-position physical/electrical layout changed'
assert len(layout) == 46 and len(set(map(tuple, expected))) == 46
original = json.loads((upstream / 'keymaps/vial/vial.json').read_text())
custom = json.loads((local / 'basch_de/vial.json').read_text())
assert custom == original
settings = (firmware / 'quantum/qmk_settings.c').read_text()
for name, source in (
    ('via_init_kb', 'quantum/via.c'),
    ('eeconfig_init_user', 'quantum/eeconfig.c'),
    ('keyboard_post_init_user', 'quantum/keyboard.c'),
):
    assert re.search(r'__attribute__\(\(weak\)\)\s+void\s+' + name + r'\(', (firmware/source).read_text()), f'{name} is no longer a weak default'
for declaration in (
    r'DECLARE_STATIC_SETTING\(\s*7\s*,\s*tapping_term\s*\)',
    r'DECLARE_STATIC_BITSETTING\(\s*22\s*,\s*tapping_v2\s*,\s*QS_tapping_permissive_hold_bit\s*\)',
    r'DECLARE_STATIC_BITSETTING\(\s*23\s*,\s*tapping_v2\s*,\s*QS_tapping_hold_on_other_key_press_bit\s*\)',
    r'DECLARE_STATIC_BITSETTING\(\s*26\s*,\s*tapping_v2\s*,\s*QS_tapping_chordal_hold_bit\s*\)',
):
    assert re.search(declaration, settings), 'Vial settings protocol used by defaults.c changed'
print('Target, GP21, full 46-position matrix, 16-layer storage, Vial identity/geometry, initialization hooks, and settings IDs verified.')
