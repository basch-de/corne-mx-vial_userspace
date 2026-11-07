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
print('Pinned target, stock byte identity, GP21, 46 positions, and Vial geometry verified.')
