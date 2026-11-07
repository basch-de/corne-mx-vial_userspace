#!/usr/bin/env python3
"""Read-only checks against the pinned manufacturer checkout."""
import json
import subprocess
import sys
from pathlib import Path

root = Path(__file__).resolve().parents[1]
firmware = Path(sys.argv[1]).resolve()
revision = subprocess.check_output(['git', '-C', str(firmware), 'rev-parse', 'HEAD'], text=True).strip()
assert revision == '63e2d6248209f1926e60daa5b950c771cf09d837', revision
upstream = firmware / 'keyboards/keebart/corne_mx_pro'
local = root / 'keyboards/keebart/corne_mx_pro/keymaps'
for name in ('keymap.c', 'config.h', 'rules.mk', 'vial.json'):
    assert (upstream / 'keymaps/vial' / name).read_bytes() == (local / 'keebart_vial_stock' / name).read_bytes(), name
assert 'SPLIT_HAND_PIN GP21' in (upstream / 'config.h').read_text()
assert (upstream / 'standard/keyboard.json').exists()
layout = json.loads((upstream / 'info.json').read_text())['layouts']['LAYOUT_split_3x6_3_ex2']['layout']
assert len(layout) == 46
assert [x['matrix'] for x in layout[40:]] == [[3,3],[3,4],[3,5],[7,5],[7,4],[7,3]]
original = json.loads((upstream / 'keymaps/vial/vial.json').read_text())
custom = json.loads((local / 'basch_de/vial.json').read_text())
assert custom == original
manifest = json.loads((root / 'docs/bindings.json').read_text())
assert [x['layer'] for x in manifest] == ['macos','num','sym','nav','win','short','numpad','sys']
for layer in manifest:
    assert len(layer['bindings']) == 46
    for i in (6,7,20,21): assert layer['bindings'][i]['qmk'] == 'KC_NO'
for i, code in ((28,'QK_RBT'),(29,'QK_BOOT'),(30,'EE_CLR'),(37,'EE_CLR'),(38,'QK_BOOT'),(39,'QK_RBT')):
    assert manifest[7]['bindings'][i]['qmk'] == code
for layer, position, code in ((0,14,'TD(0)'),(0,45,'TD(1)'),(6,38,'TD(2)'),(5,35,'KC_ASST'),(7,13,'KC_PWR')):
    assert manifest[layer]['bindings'][position]['qmk'] == code
print('Pinned target, stock byte identity, GP21, 46 positions, Vial geometry, and SYS mirroring verified.')
