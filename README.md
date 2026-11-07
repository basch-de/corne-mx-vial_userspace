# Corne MX Pro – QMK/Vial External Userspace

Ziel: **Keebart Corne MX Pro Standard, RP2040, 46 physische Tasten, keine Displays**.
Die Belegung basiert auf `corne_choc_pro.keymap` der bestehenden
ZMK-Konfiguration. Acht Layer: `macos`, `num`, `sym`, `nav`, `win`, `short`,
`numpad`, `sys`. Die vier zusätzlichen inneren MX-Tasten sind aktuell noch
unbelegt: An diesen Positionen enthält die Choc-Vorlage wegen ihrer Displays
`&none`. Ihre neue Belegung muss noch festgelegt werden.

Das Repository enthält keine QMK-Hardwaredefinitionen und keinen Firmware-Fork.
Der Build bezieht Keebarts Branch `vial`, festgesetzt auf Commit
`63e2d6248209f1926e60daa5b950c771cf09d837`.

| Keymap | Zweck |
|---|---|
| `keebart_vial_stock` | Unveränderte Hersteller-Keymap als Referenz; vier vorbelegte, 16 dynamische Layer |
| `basch_de` | Acht vorbelegte, 16 dynamische Layer, Vial-Tap/Hold-Einstellungen und normales `EE_CLR` auf SYS |

GitHub Actions baut zuerst die Herstellerreferenz und anschließend die Migration.
Nach einem Push oder über **Actions → Build Keebart Vial UF2 → Run workflow**
entstehen getrennte UF2-Artefakte und Build-Metadaten. Es gibt keine Flash-Schritte.
Workflow: [.github/workflows/build.yml](.github/workflows/build.yml).

**Tap/Hold:** Seit dem 8. November 2026 verwenden wir Vials gemeinsame Einstellungen:
250 ms, Permissive Hold und Chordal Hold an, Hold On Other Key Press aus.
Sie werden einmalig voreingestellt und können danach in Vial geändert werden.
Normale Neustarts behalten die GUI-Werte. Die frühere eigene ZMK-Timing-Nachbildung
und die Compiler-Symbolumbenennung wurden entfernt. Quick Tap startet nach
Vials Standard mit 250 ms; Flow Tap mit 0.

**Standardaktionen:** Cmd+Space/Hyper, Caps Word/SHORT und Komma/Punkt sind als
Vial Tap Dance `TD(0..2)` mit je 250 ms vorbelegt. Aktionen und Timings sind in
Vial änderbar und bleiben bei normalen Neustarts erhalten. Tap Dance wartet bei
Holds die Frist ab; frühe Unterbrechungen lösen Tap aus. Voice Command wurde durch
QMK Assistant (`KC_ASST`), Consumer Power durch System Power (`KC_PWR`) ersetzt.
Deren Wirkung auf dem Mac ist noch zu testen. Die eigene `vial.json` ist jetzt
inhaltlich identisch zur Herstellerdefinition, ohne Custom-Keycode-Einträge.

**Caps Word:** Keebart-/Vial-Standard mit fünf Sekunden Inaktivitäts-Timeout,
Großschreibung von Buchstaben und Minus zu Unterstrich; keine eigenen Overrides.

**Prüfstand:** Beide Keymaps wurden mit echten QMK-Keycodes und der Hersteller-
Layoutmatrix auf dem Host syntaxgeprüft. 32 Ereignistests gegen den unveränderten
Keebart-Tap/Hold- und Tap-Dance-Code sowie Vials originale Tap-Dance-Callbacks bestehen. Ein vollständiger RP2040-/UF2-Build und ein
GitHub-Actions-Lauf wurden noch nicht durchgeführt. Zusätzliche lokale
Buildwerkzeuge wurden in dieser Sitzung nicht zum Download freigegeben.
Hardwaretests stehen aus. Es wurde nichts geflasht oder auf der Tastatur gelöscht.

Die komplette Zuordnung aller 368 Positionen steht in [docs/bindings.json](docs/bindings.json).
Entscheidungen, Vial-Besonderheiten, verbleibende Abweichungen und Hardwaretests:
[docs/MIGRATION.md](docs/MIGRATION.md).

Lokale Prüfungen mit einem vorhandenen Checkout des festgesetzten Keebart-Commits:

```sh
python3 scripts/check_sources.py /path/to/vial-qmk-keebart
python3 tests/run.py /path/to/vial-qmk-keebart
python3 scripts/migrate_keymap.py /path/to/zmk-config/config/corne_choc_pro.keymap --check
git diff --check
```

Der Generator ohne `--check` schreibt `keymap.c` und `docs/bindings.json` neu.
Er ist für diese konkrete Quelldatei gedacht und bricht bei unbekannten Behaviors,
abweichender Layerreihenfolge oder Positionszahl ab.
