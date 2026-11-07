# Corne MX Pro – QMK/Vial External Userspace

Persönliche Firmware für die **Keebart Corne MX Pro Standard, RP2040, 46 Tasten,
keine Displays**. Sie verwendet Keebarts normale QMK-/Vial-Verarbeitung.
Die Belegung basiert ursprünglich auf der Corne-Choc-ZMK-Konfiguration und wird
jetzt direkt in [keymap.c](keyboards/keebart/corne_mx_pro/keymaps/basch_de/keymap.c)
gepflegt. Es gibt keinen Belegungsgenerator und keine Abhängigkeit vom ZMK-Repository.

| Keymap | Zweck |
|---|---|
| `basch_de` | Persönliche Belegung: acht vorbelegte von 16 dynamischen Layern |
| `keebart_vial_stock` | Unveränderte Herstellerreferenz: vier vorbelegte von 16 Layern |

Layer: `macos`, `num`, `sym`, `nav`, `win`, `short`, `numpad`, `sys`.
Vier innere Tasten sind aktuell noch unbelegt. Ihre Positionen sowie die Layer-
Zugänge stehen in [docs/PROJECT.md](docs/PROJECT.md).

## Einstellungen

Die Firmware initialisiert einmalig diese in Vial änderbaren Voreinstellungen:

- Tapping Term 250 ms, Permissive Hold und Chordal Hold an,
  Hold On Other Key Press aus; Quick Tap startet mit 250 ms, Flow Tap mit 0.
- `TD(0)`: Cmd+Space / Hyper.
- `TD(1)`: Caps Word / SHORT.
- `TD(2)`: Komma / Punkt.

Die drei Tap Dances starten ebenfalls mit je 250 ms. Ihre Zeiten sind unabhängig
vom globalen Tapping Term in Vial änderbar. Eine frühe Unterbrechung entscheidet
Tap statt Hold. Normale Neustarts behalten spätere Vial-Änderungen.

Caps Word, RGB und `EE_CLR` verwenden die Hersteller-/Vial-Standardverarbeitung.
`EE_CLR` liegt auf SYS und löst direkt beim Drücken aus. Assistant und Power
verwenden die Standardcodes `KC_ASST` und `KC_PWR`; die Hostwirkung ist noch zu testen.
Die eigene `vial.json` entspricht vollständig der Herstellerdefinition.

## Build und Prüfungen

Buildziel: `keebart/corne_mx_pro/standard`. Der
[Workflow](.github/workflows/build.yml) bezieht `Keebart/vial-qmk-keebart`, Branch
`vial`, festgesetzt auf Commit `63e2d6248209f1926e60daa5b950c771cf09d837`.
Er verwendet External Userspace, einen festgesetzten QMK-Buildcontainer und
rekursive Submodule. Die vorinstallierte Buildumgebung wird wie in Keebarts CI
direkt verwendet; eine zusätzliche Python-venv wird nicht angelegt.
Hardwaredefinitionen und QMK-Core werden nicht verändert.

Nach einem Push oder einem manuellen Workflowstart baut Actions zuerst die
Herstellerreferenz und dann die persönliche Keymap. Beide UF2-Dateien und
Build-Metadaten werden als getrennte Artefakte bereitgestellt. Es gibt keine
Flash-Schritte.

Mit einem vorhandenen Checkout des festgesetzten Keebart-Commits:

```sh
python3 scripts/check_sources.py /path/to/vial-qmk-keebart
python3 tests/run.py /path/to/vial-qmk-keebart
git diff --check
```

**Prüfstand:** Herstellerreferenz und Vial-Geometrie geprüft; beide Keymaps auf
dem Host mit echten QMK-Keycodes und Herstellerlayout kompiliert. 32 Host-
Ereignistests sowie Prüfungen der tatsächlichen Keymap-Matrix bestehen. Hardware,
USB und persistenter Speicher werden dabei simuliert. Ein vollständiger
RP2040-/UF2-Build und Hardwaretests stehen noch aus. Der gemeldete Actions-Lauf
scheiterte beim Anlegen einer Python-venv wegen fehlendem `ensurepip`; dieser
Vorbereitungsschritt wurde entfernt. Ein erfolgreicher neuer Lauf steht aus.
Es wurde nichts geflasht und kein EEPROM-Reset auf der Tastatur ausgeführt.

Technische Hinweise, Persistenz und offene Hardwaretests:
[docs/PROJECT.md](docs/PROJECT.md).
