# Projekt und Arbeitsbereich

Dieses Repository enthält QMK External Userspace für die **Keebart Corne MX Pro
Standard, kabelgebunden, mit zwei RP2040-Controllern**.

- Buildziel: `keebart/corne_mx_pro/standard`
- Eigene Keymap: `basch_de`
- Herstellerreferenz: `keebart_vial_stock`
- Kommunikation auf Deutsch; Code, Bezeichner und Quelltextkommentare auf Englisch.
- Zunächst den tatsächlichen Dateistand und lokale Änderungen prüfen.
- Keine pauschale Neuentwicklung der Belegung: Die bestehende Corne-ZMK-
  Konfiguration ist die Vorlage; ausdrücklich beauftragte MX-Anpassungen ergänzen sie.

## Aktuelle Leitlinie

Der Nutzer hat die ursprüngliche Priorität einer möglichst genauen ZMK-
Verhaltensnachbildung zugunsten der normalen Keebart-/Vial-Verarbeitung geändert.
ZMK bleibt die Belegungsvorlage, nicht die verbindliche Tap/Hold-Implementierung.

- Standardimplementierungen für Mod-/Layer-Taps, Tap Dance, Caps Word, RGB und `EE_CLR` nutzen.
- Keine eigenen Timingentscheidungen, ersetzten Tap-Ausgaben, Caps-Word-Overrides,
  Reset-Timer oder Compiler-Symbolumbenennungen wieder einführen.
- Eigene Laufzeitlogik beschränkt sich derzeit auf die einmalige Initialisierung
  von Vial-Einstellungen und Tap Dances. Die Handzuordnung ist eine explizite
  Konfiguration über den vorgesehenen QMK-Mechanismus.
- Normale Neustarts dürfen gespeicherte Vial-Änderungen nicht überschreiben.

## Hardware: wichtige Korrektur

Die **MX hat tatsächlich 46 nutzbare physische Tasten und keine Displays**.
Nicht mit der bisherigen Corne Choc Pro BT verwechseln: Dort verdecken Displays
vier der ebenfalls 46 Firmwarepositionen.

Das MX-Layout `LAYOUT_split_3x6_3_ex2` hat 14 + 14 + 12 + 6 Argumente.
Die vier zusätzlichen inneren Tasten sind:

| ZMK-Position | QMK-Matrix | Physische Lage |
|---|---|---|
| 6 | `[0,6]` | links innen, obere Reihe |
| 7 | `[4,6]` | rechts innen, obere Reihe |
| 20 | `[1,6]` | links innen, mittlere Reihe |
| 21 | `[5,6]` | rechts innen, mittlere Reihe |

Diese vier Tasten sind derzeit auf allen Layern `KC_NO`, weil die ursprüngliche
Choc-Belegung dort `&none` enthält. Ihre neue Belegung wurde noch nicht festgelegt.
Das ist ein offener Belegungspunkt, keine Hardwareeinschränkung der MX.
Vials Chordal-Hold-Handzuordnung umfasst alle 46 Positionen; die sechs Daumen
sind als handunabhängige Positionen (`*`) markiert.
Die explizite Tabelle ist beabsichtigt: QMKs Generator verwendet im festgesetzten
Branch nur das erste Herstellerlayout, hier `LAYOUT_split_3x5_3` mit 36 Positionen.
Die automatische Tabelle deckt damit nicht alle 46 MX-Tasten ab. Die sechs
Daumenausnahmen stammen aus unserer Belegung und sind kein universeller Standard.

Die rechten Matrixspalten sind gespiegelt. Daumen: links `[3,3..5]`, rechts in
Layoutreihenfolge `[7,5]`, `[7,4]`, `[7,3]`.
Die Herstellerdefinition bestätigt `SPLIT_HAND_PIN GP21`, Split-TX GP12 und
USB-VBUS GP13. Hardwaredefinitionen unverändert aus Keebarts Firmware beziehen.

## Quellen und Architektur

- Firmware: <https://github.com/Keebart/vial-qmk-keebart>, Branch `vial`.
- Untersucht und im Workflow festgesetzt:
  `63e2d6248209f1926e60daa5b950c771cf09d837`.
- ZMK-Vorlage: <https://github.com/basch-de/zmk-config>, lokal `../zmk-config`.
  Untersucht wurde Commit `17dc18d4065161066a459656a87f36f11cd4ae88`, insbesondere
  `config/corne_choc_pro.keymap`, `.conf` und `west.yml` mit ZMK `v0.3`.
- `../qmk_userspace` dient nur als Strukturreferenz; dessen Ziel ist eine Preonic.
- Das alte `../qmk_firmware` ist keine fachliche Referenz für dieses Projekt.

External Userspace beibehalten: kein vollständiger Firmware-Fork und keine
Core-/Hardwareänderungen, solange das Ziel im Userspace erreichbar ist.
Der untersuchte Keebart-Branch unterstützt `QMK_USERSPACE` nativ.
Keebart Studio und dessen Feature-Branch gehören zu einem späteren separaten Projekt.
Versionsabhängige APIs immer gegen den tatsächlich verwendeten Firmwarestand prüfen.

## Dateien und Zuständigkeiten

| Datei / Verzeichnis | Aufgabe |
|---|---|
| `qmk.json` | Beide Userspace-Buildziele |
| `.github/workflows/build.yml` | Herstellerreferenz zuerst, dann Migration; UF2-Artefakte |
| `keyboards/keebart/corne_mx_pro/keymaps/keebart_vial_stock/` | Bytegleiche Kopie der untersuchten Hersteller-Vial-Keymap |
| `keyboards/keebart/corne_mx_pro/keymaps/basch_de/keymap.c` | Generierte Layer- und Encoderbelegung |
| `keyboards/keebart/corne_mx_pro/keymaps/basch_de/behaviors.h` | Layernamen und Aliase für normale Mod-/Layer-Taps |
| `keyboards/keebart/corne_mx_pro/keymaps/basch_de/behaviors.c` | Vial-/Tap-Dance-Voreinstellungen und Handzuordnung |
| `keyboards/keebart/corne_mx_pro/keymaps/basch_de/config.h` | Timing-Voreinstellung und Vial-Identität; Layeranzahl vom Hersteller |
| `keyboards/keebart/corne_mx_pro/keymaps/basch_de/rules.mk` | Vial-Features; keine dateispezifischen Compileranpassungen |
| `scripts/migrate_keymap.py` | Erzeugt `keymap.c` und `docs/bindings.json` aus der ZMK-Vorlage |
| `scripts/check_sources.py` | Prüft Herstellerstand, unveränderte Referenz und Layoutinvarianten |
| `tests/` | Hosttests gegen Keebarts echten Tap/Hold-Code mit simulierten Hardwaregrenzen |
| `docs/MIGRATION.md` | Ausführliche Entscheidungen, Abweichungen und Hardwaretests |
| `docs/bindings.json` | Zuordnung aller 368 Layerpositionen |

Die Herstellerreferenz unverändert halten. Änderungen an der eigenen Belegung
müssen Generator und Mapping berücksichtigen, damit erneutes Generieren sie
nicht überschreibt. Insbesondere sind die vier bisher unbelegten inneren Tasten
im Generator und in der Quellenprüfung noch als `KC_NO` festgeschrieben.
Bei deren Belegung diese Prüfannahmen anpassen.
Die vierzehn logischen Spalten und die physische Reihenstruktur lesbar ausrichten.

## Belegung und Verhalten

Layerreihenfolge: `MACOS=0`, `NUM=1`, `SYM=2`, `NAV=3`, `WIN=4`, `SHORT=5`,
`NUMPAD=6`, `SYS=7`.

- **Backspace halten aktiviert NUM; Tab halten aktiviert WIN.** Die Quelldatei
  ist hier maßgeblich, nicht ältere Zusammenfassungen.
- `WIN` enthält die originalen Ctrl+Alt-Fensteraktionen, kein Windows-Basislayout.
- NUMPAD verwendet normale Zifferncodes. Die Toggle-Wege erhalten.
- Sonderzeichen und Shortcuts als originale HID-Codes und Modifier übertragen;
  aus dem Namen `basch_de` kein deutsches QWERTZ-Hostlayout ableiten.
- Die ZMK-Vorlage definiert keine expliziten Combos.

Seit dem Nutzerauftrag vom 8. November 2026 gelten gemeinsame Vial-Einstellungen:
Tapping Term 250 ms, Permissive Hold an, Chordal Hold an, Hold On Other Key Press aus.
250 ms ist Pascals empfohlener HRM-Ausgangspunkt, kein allgemeiner Tap-Dance-
Empfehlungswert. Vials globaler Wert gilt auch für normale Layer-Taps; es gibt
keine separaten Timinggruppen. Ohne unseren Override wäre `TAPPING_TERM` 200 ms.
Diese Werte werden auf dem USB-Master bei abweichender User-Versionskennung oder
ungültigem Vial-Speicher über `qmk_settings_set()` initialisiert. Die Kennung wird
nur nach erfolgreichen Schreibvorgängen gespeichert. Normale Neustarts dürfen
spätere Vial-Änderungen nicht überschreiben. `via_init_kb()` merkt sich den
Vial-Gültigkeitszustand vor der Coreinitialisierung; `keyboard_post_init_user()`
setzt die Werte nach der Initialisierung. `eeconfig_init_user()` leert nur die
User-Kennung, wenn QMK selbst seine Einstellungen initialisiert.

Alle normalen MT/LT-Tasten verwenden Vials native gemeinsame Regeln. Keine eigenen
Timing-/Chordal-Hold-Hooks und keine Symbolumbenennung mehr einführen, solange
keine neue Anforderung dies verlangt. Quick Tap folgt Vials Initialwert von
`TAPPING_TERM` (250 ms) und seiner Release-zu-Press-Messung; Flow Tap startet bei 0.
Die früheren ZMK-Gruppen mit 280/200 ms und ihren separaten Quick-Tap-Fenstern
werden bewusst nicht mehr nachgebildet. Auch die Positionsentscheidung erfolgt
nach Standard-Chordal-Hold, nicht nach ZMKs `hold-trigger-on-release`.

Die drei kombinierten Aktionen verwenden auf Nutzerauftrag Standard-Vial-Tap-Dance:

| Code | Tap | Hold | Anfangsfrist |
|---|---|---|---|
| `TD(0)` | `LGUI(KC_SPC)` | `HYPR(KC_NO)` | 250 ms |
| `TD(1)` | `CW_TOGG` | `MO(SHORT)` | 250 ms |
| `TD(2)` | `KC_COMM` | `KC_DOT` | 250 ms |

Die drei Fristen verwenden aktuell ebenfalls `TAPPING_TERM`. Das ist unsere
Voreinstellung, keine zusätzliche Empfehlung von Pascal. Vial speichert die Zeit
pro Eintrag als `custom_tapping_term`; spätere Änderungen des globalen Timings
ändern diese Einzelwerte nicht automatisch. Separate Firmware-Voreinstellungen
können im jeweils letzten Feld von `default_tap_dances` angegeben werden.

Die Voreinträge stehen in `default_tap_dances` in `behaviors.c` und werden bei der
Erstinitialisierung über `dynamic_keymap_set_tap_dance()` gespeichert. Die gemeinsame
User-Versionskennung wird erst nach erfolgreichen QMK-Settings- und Tap-Dance-
Schreibvorgängen gesetzt. Normale Neustarts erhalten danach auch Änderungen an
Tap-Dance-Aktionen und ihren individuellen Fristen. Doppeltipp-/Tap-Hold-Felder sind
0 und verwenden Vials eingebaute Wiederholungs-/Fallback-Regeln. Keine eigenen
Tap-Dance-Callbacks oder parallelen `tap_dance_actions` definieren.

Tap Dance verwendet eigene Entscheidungen: Der erste Tap wartet auf Timeout oder
Unterbrechung; eine andere Taste vor der Frist entscheidet Tap statt Hold.
SHORT/Hyper deshalb erst nach Ablauf der Frist nutzen. Diese drei Tasten folgen
nicht den globalen Permissive-/Chordal-Hold-Einstellungen der HRMs.

Die früheren F24-/Layer-0-Träger und deren Ausgabebehandlung sind entfernt.
`QUOT_CSA` ist weiterhin ein gewöhnlicher Ctrl+Alt+Shift-/Apostroph-Mod-Tap.
Die bisherigen Custom-HID-Aktionen wurden durch Standard-QMK ersetzt:
`KC_ASST` (Consumer AL Assistant, 0x1CB) statt ZMK Voice Command (0xCF) und
`KC_PWR` (Generic Desktop System Power, 0x81) statt Consumer Power (0x30).
Die Hostwirkung dieser bewusst geänderten Befehle ist noch zu testen.
Es gibt keine eigenen Custom Keycodes mehr; beide `vial.json` entsprechen der
unveränderten Herstellerdefinition.

Die Belegung stammt aus ZMK; die Tap/Hold-Entscheidungen wurden ausdrücklich auf
Vial umgestellt. Originale ZMK-Flavors werden nicht mehr zugesichert.
Details und Hardwareprüfungen stehen in `docs/MIGRATION.md`.

## Vial-Kompatibilität

Vial-UID, Unlock-Kombination und Geometrie des Herstellers erhalten. Die eigene
Keymap verwendet Keebarts Standard von 16 dynamischen Layern, davon acht vorbelegt.
Die Herstellerreferenz hat ebenfalls 16, davon vier vorbelegt.
`DYNAMIC_KEYMAP_LAYER_COUNT` nicht auf acht überschreiben.
Die Encoder-Map muss genauso viele vorbelegte Layer wie die Keymap besitzen.

`QMK_SETTINGS = yes` ist aktiv. Vials eigene Timing-Hooks, Tap-Dance-Timings und
Shift-Invertierung für Caps Word bleiben unverändert. Es gibt keine zusätzlichen
`FILE_SPECIFIC_CFLAGS`. Caps Word verwendet ebenfalls unverändert den Standard:
kein eigener `caps_word_press_user()` und kein `CAPS_WORD_IDLE_TIMEOUT`-Override.
Der festgesetzte Branch beendet Caps Word nach fünf Sekunden Inaktivität und
shiftet neben Buchstaben auch Minus zu Unterstrich. Vials Shift-Invertierung bleibt aktiv.
Die Protokoll-IDs 7/22/23/26 für die Initialisierung stammen aus dem festgesetzten
`quantum/qmk_settings.c`; bei Firmwareupdates prüfen. Ein explizites Zurücksetzen
über Vial kann dessen eigene Resetwerte herstellen, nicht automatisch unsere
Erststartvoreinstellungen. Kein Reset bei normalen Neustarts.

Dieser Branch verwendet eine zufällige Build-ID als EEPROM-Gültigkeitskennung.
Ein anderes Image kann beim späteren Start Vial-Einstellungen neu initialisieren.
Keine pauschalen zusätzlichen EEPROM-Reset-Hooks einbauen.

## Firmware-Sicherheit und SYS

Ausdrückliche Vorgaben des Nutzers:

- Keine Firmware automatisch flashen.
- Keine EEPROM-Resets oder vollständigen Flash-Löschungen ausführen.
- Zunächst Quellcode und Buildsystem erstellen; UF2-Dateien über Actions bereitstellen.

SYS enthält unten links außen nach innen `QK_RBT`, `QK_BOOT`, `EE_CLR`, rechts
spiegelbildlich. Auf ausdrücklichen Nutzerauftrag verwendet `EE_CLR` den normalen
QMK-Handler: Auslösen beim Drücken, ohne eigene Haltedauer oder Bestätigung.
Kein eigener `process_record_user()`-/`layer_state_set_user()`-Reset-Guard.
Der SYS-Layer ist der Zugangsschutz; der frühere Drei-Sekunden-Timer ist entfernt.
Die Aktionen laufen auf dem USB-Master. Eine rechts platzierte Bootloadertaste
adressiert nicht automatisch den rechten Controller; es gibt keine eigene Split-RPC.
Beide Hälften müssen später unabhängig auf Bootloader-Erreichbarkeit geprüft werden.

## Prüfungen und bekannter Stand

Der Workflow pinnt Firmware-Commit und Hersteller-Buildcontainer, berücksichtigt
rekursive Submodule und erzeugt getrennte Artefakte sowie Build-Metadaten.
Bitidentische Builds sind wegen Build-ID und beweglicher Python-Abhängigkeiten
nicht zugesichert.

Bei vorhandenem Checkout des festgesetzten Keebart-Commits:

```sh
python3 scripts/check_sources.py /path/to/vial-qmk-keebart
python3 tests/run.py /path/to/vial-qmk-keebart
python3 scripts/migrate_keymap.py /path/to/zmk-config/config/corne_choc_pro.keymap --check
git diff --check
```

Auch neue, noch unversionierte Dateien auf Whitespacefehler prüfen.
Nach reinen Dokumentationsänderungen sind keine Firmwaretests erforderlich.

Prüfstand nach Umstellung auf Vial-Einstellungen am 8. November 2026:

- Python 3.9 über Apples Command Line Tools ist lokal vorhanden; die Skripte liefen damit.
- Herstellerreferenz byteweise geprüft; alle 368 Positionen abgeglichen.
- Beide Keymaps auf dem Host mit echten QMK-Keycodes und Herstellerlayout syntaxgeprüft.
- 32 Host-Ereignistests gegen unveränderte Keebart-`action_tapping.c` und
  `process_tap_dance.c` sowie Vials originale Tap-Dance-Callbacks bestanden;
  echte Vial-/QMK-Settings-Timing-Hooks werden ohne Symbolumbenennung gelinkt.
  Initialisierung, Erhalt von GUI-Werten, USB-Master, Schreibfehler, Tap Dance
  (Tap, Hold, Unterbrechung, Doppeltipp, GUI-Änderung) sind getestet;
  die Speicher-/Hardwaregrenzen sind dabei simuliert.
- Kein vollständiger ARM-/UF2-Build und kein GitHub-Actions-Lauf bestätigt.
  Der Download zusätzlicher lokaler Buildwerkzeuge wurde in der Einrichtungssitzung abgelehnt.
- Hardwaretests und Belegung der vier zusätzlichen Tasten offen.
- Keine Firmware geflasht, keine Tastatureinstellungen gelöscht, nichts gepusht.

Ein erfolgreicher Hosttest ist kein erfolgreicher Firmwarebuild. Einen Build
nur mit vorliegendem Ergebnis für den konkreten Änderungsstand als erfolgreich
melden. Bestätigte spätere Build- und Hardwareergebnisse in der Dokumentation
aktualisieren; historische Prüfergebnisse nicht auf neue Änderungen übertragen.
