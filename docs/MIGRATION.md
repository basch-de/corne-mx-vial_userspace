# Migration und Prüfstand

## Verbindliche Quellen

Untersuchter Stand vom 7. November 2026:

| Quelle | Verwendeter Stand |
|---|---|
| [Keebart-Firmware, Branch vial](https://github.com/Keebart/vial-qmk-keebart/tree/63e2d6248209f1926e60daa5b950c771cf09d837) | `63e2d6248209f1926e60daa5b950c771cf09d837`, per temporärem Git-Checkout geprüft |
| [ZMK-Konfiguration](https://github.com/basch-de/zmk-config/tree/17dc18d4065161066a459656a87f36f11cd4ae88) | Lokaler, sauberer Checkout `17dc18d4065161066a459656a87f36f11cd4ae88`; `config/corne_choc_pro.keymap`, `.conf`, `west.yml` |
| [ZMK v0.3 Hold/Tap-Code](https://github.com/zmkfirmware/zmk/blob/v0.3/app/src/behaviors/behavior_hold_tap.c) | Die in `west.yml` festgelegte Version; insbesondere `is_quick_tap`, `decide_balanced` und `decide_positional_hold` |
| [ZMK Standardbehaviors](https://github.com/zmkfirmware/zmk/tree/v0.3/app/dts/behaviors) | `mt`: hold-preferred, 200 ms; `lt`: tap-preferred, 200 ms; Quick Tap standardmäßig deaktiviert |
| Bestehendes `../qmk_userspace` | Nur Struktur und Workflow als Orientierung; dessen Preonic-Belegung wurde nicht übernommen |

Die Ziel-Keymap hat keine zusätzlichen benutzerdefinierten Includes. Sie bindet
ZMK-Standardbehaviors und Keycode-, Bluetooth-, RGB- und Stromversorgungsdefinitionen
ein. Es gibt keine expliziten ZMK-Combos: „Tastenkombinationen“ sind hier
Modifierakkorde und Tap/Hold-Unterbrechungen.

## Hardware und Userspace

`keyboards/keebart/corne_mx_pro/standard/keyboard.json` bestätigt das Buildziel.
Der RP2040, USB-VBUS GP13, serielle Split-Verbindung GP12 und die Seitenerkennung
`SPLIT_HAND_PIN GP21` stammen unverändert vom Hersteller.

Die MX-Zielhardware hat **46 physische, nutzbare Tasten und keine Displays**.
Die Layoutmatrix `LAYOUT_split_3x6_3_ex2` hat dieselbe Argumentreihenfolge wie ZMK:
14 + 14 + 12 + 6 Positionen. QMK verwendet acht Matrixzeilen und sieben Spalten.
Die rechten Spalten sind gespiegelt; die Daumen liegen auf `[3,3..5]` und
`[7,5..3]`. ZMK-Positionen 6, 7, 20 und 21 sind in der Choc-Vorlage wegen deren
Displays deaktiviert. Auf der MX sind dort vier zusätzliche Tasten vorhanden:
links `[0,6]` und `[1,6]`, rechts `[4,6]` und `[5,6]`. Die bisherige Übertragung
setzt diese auf allen Layern noch auf `KC_NO`; damit sind aktuell 42 der 46
physischen Tasten belegt. Die Belegung dieser vier Tasten ist noch festzulegen.

Keebarts `builddefs/build_keyboard.mk` und `lib/python/qmk/userspace.py`
unterstützen `QMK_USERSPACE` nativ. Die Keymaps liegen im externen Verzeichnis
`keyboards/keebart/corne_mx_pro/keymaps/`; es wird nichts in den Firmwarebaum kopiert.
Die eindeutigen Keymapnamen vermeiden eine Kollision mit dessen `vial`-Keymap.

## Build-Integration

`qmk.json` enthält beide Targets. Der eigene Workflow verwendet den unveränderlichen
Container-Digest aus Keebarts eigener CI, den festgesetzten Commit des `vial`-Branches
und rekursive Git-Submodule. Die Herstellerreferenz wird zuerst kompiliert und
hochgeladen. Nur danach wird die Migration kompiliert und separat hochgeladen.
Pull Requests laufen mit Leserechten; es gibt keine Veröffentlichung als Release.

Der Workflow zeichnet Firmware-Commit, Userspace-Commit, Submodule, Compiler und
Python-Pakete auf. Firmware und Container sind festgesetzt; Actions `@v4` und
Python-Paketauflösung bleiben beweglich. Bitidentische Builds werden deshalb nicht
behauptet. Zusätzlich erzeugt dieser Vial-Branch mit `util/build_id.py` eine
zufällige 24-Bit-Build-ID. Die Hardwarequellen werden nicht für deterministische
Build-IDs verändert.

Buildstatus: **kein vollständiger ARM-/UF2-Build und kein Online-Lauf bestätigt**.
Der lokale Download von ARM-Compiler, Python-Paketen und Submodulen wurde abgelehnt.
Es wurden keine Commits gepusht und keine GitHub-Workflows gestartet. Der nächste
Integrationsnachweis ist ein grüner Lauf beider Buildschritte. Die Hosttests
ersetzen insbesondere den Link mit ChibiOS und die Prüfung der Firmwaregröße nicht.

## Layer und Aktionen

Die JSON-Datei [bindings.json](bindings.json) enthält für jeden Layer die
ZMK-Bindung und ihre QMK-Entsprechung an jeder Position, einschließlich der
bewussten Abweichungen auf SYS. Sie dokumentiert die Tastenaktionen, nicht eine
Gleichheit der Tap/Hold-Entscheidungen.

| Nr. | Layer | Inhalt / Zugang |
|---|---|---|
| 0 | macos | Originale Buchstaben, HRMs und Daumenaktionen |
| 1 | num | Zahlenreihe und F1–F12; **Backspace halten** |
| 2 | sym | US-HID-Symbolcodes und NUMPAD-Toggle; Space halten |
| 3 | nav | macOS-Navigation und Bearbeitungsaktionen; Escape halten |
| 4 | win | Originale Ctrl+Alt-Fensteraktionen; Tab halten; kein Windows-Basislayout |
| 5 | short | macOS-/App-Shortcuts; Gleichheitszeichen oder Caps-Word-Taste halten |
| 6 | numpad | Normale Zahlen, keine Num-Lock-abhängigen Keypadcodes; Enter halten oder Toggle |
| 7 | sys | SYS/Grave halten; Reset, Bootloader, EEPROM, RGB, System Power |

`num` liegt in der tatsächlichen Quelle auf Backspace, entgegen der Tab-Angabe
in einer älteren Zusammenfassung. Alle `TG(NUMPAD)`-Lock-/Unlock-Wege bleiben
erhalten. Keine zusätzlichen automatischen Layer wurden hinzugefügt.

| ZMK | QMK |
|---|---|
| `&kp`, verschachtelte `LC/LA/LG/LS` | Native Keycodes und Ctrl/Alt/GUI/Shift-Kombinationen |
| `&none`, `&trans`, `&tog` | `KC_NO`, `KC_TRNS`, `TG(...)` |
| `hml` / `hmr` | Native Mod-Taps mit gemeinsamen Vial-Einstellungen |
| `hlt` | Native Layer-Taps mit gemeinsamen Vial-Einstellungen |
| `rhlt` | Native Layer-Taps mit gemeinsamen Vial-Einstellungen |
| `rlt` | Native Layer-Taps mit gemeinsamen Vial-Einstellungen |
| `lt SHORT EQUAL` | `LT(SHORT, KC_EQL)` mit gemeinsamen Vial-Einstellungen |
| Hyper halten / Cmd+Space tippen | Vial `TD(0)`: Tap Cmd+Space, Hold Hyper |
| Ctrl+Alt+Shift halten / Apostroph tippen | `QUOT_CSA`, nativer Mod-Tap |
| SHORT halten / Caps Word tippen | Vial `TD(1)`: Tap Caps Word, Hold `MO(SHORT)` |
| Punkt halten / Komma tippen | Vial `TD(2)`: Tap Komma, Hold Punkt |
| Consumer Voice Command | Standard `KC_ASST`: AL Assistant (0x1CB), bewusst andere Usage |
| Consumer Power | Standard `KC_PWR`: Generic Desktop System Power (0x81), bewusst andere Usage |
| RGB-Unterglow-Bedienung | Entsprechende `RM_*`-Aktionen für die verbaute RGB-Matrix |
| `sys_reset`, `bootloader` | `QK_RBT`, `QK_BOOT` |
| Bluetoothprofile / Pairing löschen | `KC_NO`: kabelgebundene Hardware ohne Bluetooth |
| Externe Stromversorgung umschalten | `KC_NO`: keine übertragbare ZMK-Stromschaltung |
| `studio_unlock` | `KC_NO`: ZMK Studio gehört nicht zum Vial-Projekt |

Die Sonderzeichen wurden als originale US-HID-Codes einschließlich Modifiern
übernommen. Die resultierenden Zeichen hängen weiterhin vom Hostlayout ab;
eine Umdeutung auf deutsches QWERTZ wurde nicht vorgenommen.

## Tap/Hold: gemeinsame Vial-Einstellungen

Am 8. November 2026 wurde auf Nutzerauftrag von der eigenen ZMK-Nachbildung
auf Vials native Tap/Hold-Verarbeitung umgestellt. Die normalen Mod-/Layer-Taps verwenden gemeinsame GUI-Einstellungen. Die drei
später auf Standard-Tap-Dance umgestellten Kombinationen haben separate Fristen.

| Einstellung | Firmware-Voreinstellung |
|---|---|
| Tapping Term | 250 ms |
| Permissive Hold | an |
| Chordal Hold | an |
| Hold On Other Key Press | aus |
| Quick Tap | Vial-Standard: 250 ms ab Loslassen |
| Flow Tap | Vial-Standard: 0 ms (aus) |

Die Handzuordnung umfasst alle 46 physischen Positionen; alle sechs Daumen sind
mit `*` von der Gegenhandregel ausgenommen. Normales Chordal Hold kann gleichhändige
Kombinationen bereits beim Drücken als Tap entscheiden. Die bisherigen separaten
ZMK-Flavors, 280/200-ms-Fristen und Press-zu-Press-Quick-Tap-Fenster entfallen.
Auch die ursprünglichen hold-preferred-Daumentasten nutzen jetzt gemeinsame Regeln.

`QMK_SETTINGS = yes` ist aktiv. Eigene `get_tapping_term`, `get_quick_tap_term`,
`get_permissive_hold`, `get_hold_on_other_key_press` und `get_chordal_hold` wurden
entfernt. Ebenso entfallen die Symbolumbenennung von Vials Timing-Funktion und
beide dateispezifischen Compileroptionen. Vials ursprüngliche Timing-Hooks werden
unverändert verwendet, einschließlich dynamischer Tap-Dance-Timings.

## Vial: Erhalt und Grenzen

UID `89 36 2A C7 FA D8 89 45`, Matrixgeometrie, Layoutoptionen und Unlock-Kombination
`[0,0] + [0,1]` bleiben wie beim Hersteller. Beide Positionen müssen für die
Vial-Entsperrung physisch gehalten werden; auf der Migration sind dies SYS/Grave
und Q. Beide Keymaps verwenden unverändert den Herstellerstandard von 16 dynamischen
Layern. Die eigene Keymap belegt acht davon vor; die weiteren acht stehen in Vial
zur Verfügung. Die Herstellerreferenz belegt vier Layer vor.

VIA/Vial, VialRGB, Encoder-Map, Caps Word und Repeat Key bleiben aktiv.
Die vier Hersteller-Encoderbelegungen sind auf allen acht Layern vorhanden,
auch wenn keine physischen Encoder eingebaut sind. Tap Dance, Combos und
Key Overrides bleiben als Vial-Funktionen verfügbar; es werden keine neuen
Belegungskombos voreingestellt.

Die Vial-Seite **QMK Settings** ist verfügbar. Die vier Firmware-Voreinstellungen
werden über die Protokoll-IDs 7, 22, 23 und 26 aus dem festgesetzten Branch gesetzt.
Vials Init-Code übernimmt zwar `TAPPING_TERM`, initialisiert die Verhaltensschalter
aber unabhängig von QMK-Makros mit 0. Deshalb ist eine kleine Initialisierung nötig:

1. `via_init_kb()` merkt sich, ob Vials Speicher vor der Coreinitialisierung ungültig war.
2. `keyboard_post_init_user()` setzt die Werte nur auf dem USB-Master und nur bei
   neuer/ungültiger Vial-Konfiguration oder fehlender User-Versionskennung.
3. Anschließend werden die drei Standard-Tap-Dance-Einträge über
   `dynamic_keymap_set_tap_dance()` gespeichert. Erst nach erfolgreichen
   QMK-Settings- und Tap-Dance-Schreibvorgängen wird die Kennung im
   User-EEPROM gespeichert. Die Belegung oder der gesamte Speicher werden dabei
   nicht durch eigenen Code zurückgesetzt.
4. Bei normalen Neustarts bleiben alle späteren Vial-Änderungen erhalten,
   einschließlich individueller Tap-Dance-Aktionen und Timings.

Ein explizites Zurücksetzen der Einstellungen in Vial verwendet dessen eigene
Resetwerte; die Initialisierung hier ist kein Ersatz für einen veränderten
Vial-Reset-Hook. Ein neues Image mit anderer Build-ID kann die Erstinitialisierung
wieder auslösen. Die Init-Reihenfolge ist gegen den gepinnten Code geprüft, aber
noch nicht durch einen vollständigen Firmwarebuild oder Hardwarestart bestätigt.

## Standardaktionen und Tap Dance

Die früheren ersetzten F24-/Layer-0-Taps wurden auf Nutzerauftrag entfernt.
Die Keymap verwendet jetzt ausschließlich normale QMK-/Vial-Codes:

| Eintrag | Tippen | Halten | Frist |
|---|---|---|---|
| `TD(0)` | Cmd+Space | Hyper (`HYPR(KC_NO)`) | 250 ms |
| `TD(1)` | Caps Word (`CW_TOGG`) | SHORT (`MO(SHORT)`) | 250 ms |
| `TD(2)` | Komma | Punkt | 250 ms |

Vial liefert die gesamte Auswertung; es gibt keine eigenen Tap-Dance-Callbacks.
Die Einträge werden einmalig vorbelegt und sind anschließend in der Vial-
Tap-Dance-Seite direkt änderbar. Die individuellen Tap-Dance-Fristen sind
unabhängig vom globalen HRM-Tapping-Term. Die Felder für Doppeltipp und Tap-Hold
bleiben leer (0); Vial verwendet seine Standardfallbacks: Doppeltipp wiederholt
normalerweise die Tap-Aktion, Tap-then-Hold sendet zunächst Tap und hält dann Hold.
Bei Caps Word kann die wiederholte Toggle-Aktion dadurch wieder ausschalten.

Ein kurzer Tap wird normalerweise erst bei Fristablauf oder Unterbrechung
aufgelöst. Bei Unterbrechung des ersten Taps durch eine andere Taste vor Fristablauf
entscheidet Vial Tap statt Hold. Für SHORT-/Hyper-Akkorde deshalb zunächst die
Frist abwarten. Normale MT/LT-Tasten, einschließlich der HRMs, bleiben beim
zuvor eingerichteten Permissive-/Chordal-Hold-Verhalten.

Auch die bisherigen Custom-Keycodes für Voice Command und Consumer Power wurden
entfernt. `KC_ASST` sendet AL Assistant (Consumer 0x1CB), nicht ZMK Voice Command
(0xCF). `KC_PWR` sendet System Power (Generic Desktop 0x81), nicht Consumer Power
(0x30). Diese Standardbefehle sind bewusst andere HID-Aktionen; die Wirkung auf
dem Mac ist noch nicht bestätigt. Beide `vial.json` sind nun unveränderte Kopien
der Herstellerdefinition ohne eigene `customKeycodes`.

Vial-Belegungen liegen persistent im emulierten EEPROM. Der Compilezeit-Keymap-
Array dient als Initialwert bei ungültigem Speicher. Dieser Branch erzeugt pro
Build eine zufällige Build-ID und verwendet sie als EEPROM-Gültigkeitskennung.
Ein neu gebautes Image kann daher beim späteren ersten Start die dynamischen
Vial-Einstellungen neu initialisieren. Änderungen an der dynamischen Layeranzahl
oder `QMK_SETTINGS` können außerdem das Speicherlayout ändern. Es gibt keinen
zusätzlichen pauschalen Reset-Hook im Userspace.
Nach künftigen Vial-Änderungen vor Firmwarewechseln die Belegung exportieren.
Derzeit wurden weder Firmware gestartet noch EEPROM-Einstellungen verändert.

## SYS und beide Controller

In der unteren Hauptreihe liegen links außen nach innen `QK_RBT`, `QK_BOOT`,
`EE_CLR`; rechts von innen nach außen `EE_CLR`, `QK_BOOT`, `QK_RBT`.
Die Quelle hatte Reset und Bootloader bereits an diesen spiegelbildlichen Stellen.
Die EEPROM-Funktion verwendet bisher unbelegte Positionen.

Auf ausdrücklichen Nutzerauftrag verwendet `EE_CLR` wieder den unveränderten
QMK-Handler: Reset beim Tastendruck, ohne eigene Haltedauer oder Bestätigung.
Der zusätzliche Drei-Sekunden-Timer sowie die eigenen Record-/Layer-State-Hooks
sind entfernt. Die Taste liegt weiterhin ausschließlich auf dem SYS-Layer.
Es wurde kein Reset an einer angeschlossenen Tastatur ausgeführt.

QMK führt Keymap-Aktionen auf dem USB-Master aus. Die rechte Bootloadertaste
adressiert deshalb nicht automatisch den rechten Controller. Es gibt keine neue
Split-RPC für Bootloader oder EEPROM. Keebarts Split-Watchdog ist kein Nachweis,
dass beide Hälften gemeinsam in den Bootloader wechseln. Die GPIO-Seitenerkennung
macht getrennte linke/rechte Buildvarianten normalerweise unnötig; das muss mit
USB an beiden Hälften geprüft werden.

## Bewusste Abweichungen und offene Tests

Die Belegung ist aus ZMK übertragen; das Tippverhalten wird seit der Umstellung
bewusst durch Vial bestimmt. Die Hosttests prüfen konkrete Ereignisfolgen,
einschließlich der Änderung des globalen Timings und der Erstinitialisierung.
Hardware, EEPROM-Speicher und USB sind im Hosttest simuliert.

Die originalen ZMK-Regeln sind keine Vorgabe mehr für Tap/Hold-Entscheidungen.
HRM-Ketten, Rollen, Daumenaktionen, Quick Tap und GUI-Änderungen müssen an der
Tastatur nach persönlichem Tippgefühl geprüft werden. Scanrate und Split-Latenz
sind im Hosttest nicht abgebildet.

Caps Word schaltet per Tap um und verwendet auf Nutzerauftrag vollständig die
Standardeinstellungen: fünf Sekunden Idle-Timeout, Buchstaben und Minus werden
geshiftet (Minus wird Unterstrich). Eigener Callback und Timeout-Override sind
entfernt. Vials standardmäßige Shift-Invertierung bleibt aktiv.
Weitere Unterschiede gegenüber ZMK (etwa Modifierbehandlung und Beenden durch
Shortcuts) bleiben bestehen.

`.conf`-Einstellungen für Bluetoothname, Akkustromversorgung, Funk-/Deep-Sleep
und ZMK-Displays werden nicht blind auf die kabelgebundene MX-Hardware übertragen.
Die originalen Keebart-RGB-Startwerte, Helligkeitsgrenze, Schrittweiten und
Suspendfunktion bleiben erhalten. Der ZMK-Auto-Off nach fünf Minuten und seine
RGB-Defaults (20 %, 200°, 98 %) sind keine bestätigte Entsprechung. RGB-Matrix-
Effekte haben andere Namen/Reihenfolgen als ZMK-Unterglow. Hardware-/Anzeige-
Code und dessen Standardverhalten bleiben unverändert.

Vor dem Einsatz an der Hardware:

1. Beide UF2-Builds in Actions bestätigen; Build-Provenienz aufbewahren.
2. Jede Hälfte einzeln per USB starten und ihren physischen Bootloaderzugang
   unabhängig prüfen. TRRS nur im stromlosen Zustand umstecken.
3. Mit verbundenem Split USB links und rechts testen; Seitenerkennung, alle
   46 physischen Positionen, RGB und gegebenenfalls Encoder kontrollieren;
   die vier zusätzlichen inneren Tasten benötigen zuvor eine Belegung.
4. HRMs: gleiche/gegensätzliche Hand, Rollen, verschachtelte Taps, mehrere HRMs,
   alle sechs Daumen, 250-ms-Grenze und lange Holds vergleichen.
5. Wiederholungen vor/nach dem in Vial gewählten Quick-Tap-Fenster seit Loslassen testen,
   mit kurzen und langen ersten Taps und zusätzlichen Tasten dazwischen.
6. Alle Layer, NUMPAD-Toggle, die drei Tap-Dance-Einträge (Tap, Hold, frühe
   Unterbrechung, Doppeltipp und Wiederholung) sowie Apostroph/Meh testen.
   Die geänderten Standardaktionen `KC_ASST` und `KC_PWR` am Mac prüfen.
7. Vial-Erkennung, Entsperren, 16 dynamische Layer (acht vorbelegt), Tap Dance, Belegungsexport und
   persistente Neubelegung nach normalem Neustart prüfen.
8. SYS-Zugänge zunächst nur mit Neustart/Bootloader prüfen. `EE_CLR` hat keinen
   zusätzlichen Timer und darf nur bei bewusst gewünschtem Zurücksetzen verwendet
   werden. Tatsächliches Löschen gehört nicht zu dieser Umsetzung.

## Durchgeführte Prüfungen

- Unveränderte Herstellerreferenz byteweise mit dem Keebart-Checkout verglichen.
- Layerfolge, alle 368 Bindungen, vier unbelegte innere Positionen aus der Choc-Vorlage, Daumenmatrix und SYS-Spiegelung geprüft.
- Beide C-Keymaps mit echten Hersteller-Layoutmakros und QMK-Keycodes syntaxgeprüft.
- 32 Host-Szenarien gegen die unveränderte `action_tapping.c` und
  `process_tap_dance.c` sowie Vials originale Tap-Dance-Callbacks bestanden,
  mit echten Vial-/QMK-Settings-Hooks ohne Symbolumbenennung. Initialisierung,
  Erhalt von GUI-Werten, Masterbeschränkung, Schreibfehler und Tap Dance
  (Tap, Hold, Unterbrechung, Doppeltipp und GUI-Änderung) wurden getestet.
- Generator mit `--check` und `git diff --check` ausgeführt.
- Kein Flash, EEPROM-Reset oder vollständiges Flash-Löschen durchgeführt.
