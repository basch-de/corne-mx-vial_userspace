# Konfiguration und offene Prüfungen

## Hardware und Quellen

Ziel ist ausschließlich die **Keebart Corne MX Pro Standard mit 46 physischen
Tasten und zwei RP2040-Controllern, ohne Displays**. Buildziel:
`keebart/corne_mx_pro/standard`.

Hardwaredefinitionen stammen unverändert aus
[Keebarts Vial-Branch](https://github.com/Keebart/vial-qmk-keebart/tree/63e2d6248209f1926e60daa5b950c771cf09d837).
Bestätigt sind Seitenerkennung GP21, Split-TX GP12 und USB-VBUS GP13.
Das Layout `LAYOUT_split_3x6_3_ex2` enthält 14 + 14 + 12 + 6 Positionen.
Die rechten Matrixspalten sind gespiegelt; die Daumen liegen links auf `[3,3..5]`
und rechts in Layoutreihenfolge auf `[7,5]`, `[7,4]`, `[7,3]`.

Noch unbelegt sind die vier inneren Tasten `[0,6]`, `[1,6]`, `[4,6]`, `[5,6]`.
Sie sind auf der MX nutzbar. Die ursprüngliche
[ZMK-Belegung](https://github.com/basch-de/zmk-config/tree/17dc18d4065161066a459656a87f36f11cd4ae88)
ließ diese Positionen wegen der Displays der Choc-Hardware frei. Es gibt keine
laufende Abhängigkeit von dieser Quelle; `keymap.c` ist jetzt maßgeblich.

## Dateien pflegen

| Datei in `keyboards/keebart/corne_mx_pro/keymaps/basch_de/` | Inhalt |
|---|---|
| `keymap.c` | Belegung und Encoder-Map direkt bearbeiten |
| `behaviors.h` | Layernamen und Aliase für normale Mod-/Layer-Taps |
| `behaviors.c` | Vial-Voreinstellungen, drei Tap-Dance-Einträge und Handzuordnung |
| `config.h` | Anfänglicher Tapping Term und Hersteller-Vial-Identität |
| `rules.mk` | Aktivierte Standardfeatures und zusätzliche Quelldatei |
| `vial.json` | Unveränderte Herstellergeometrie für Vial |

Die Keymap verwendet acht vorbelegte und 16 verfügbare dynamische Layer. Auf allen
acht vorbelegten Layern ist die vierteilige Hersteller-Encoder-Map vorhanden,
auch wenn keine Encoder eingebaut sind.

| Nr. | Layer | Zugang / Zweck |
|---|---|---|
| 0 | MACOS | Basisbelegung mit HRMs |
| 1 | NUM | Backspace halten; Zahlenreihe und F-Tasten |
| 2 | SYM | Space halten; Symbole und NUMPAD-Toggle |
| 3 | NAV | Escape halten; Navigation und Bearbeitung |
| 4 | WIN | Tab halten; Ctrl+Alt-Fensteraktionen, kein Windows-Basislayout |
| 5 | SHORT | Gleichheitszeichen halten oder `TD(1)` halten; Shortcuts |
| 6 | NUMPAD | Enter halten oder Toggle; normale Zifferncodes ohne Num-Lock-Abhängigkeit |
| 7 | SYS | Grave halten; Neustart, Bootloader, EEPROM und RGB |

Die Keymap verwendet US-HID-Codes. Die ausgegebenen Zeichen und Shortcuts hängen
vom Hostlayout ab. Aus `basch_de` kein deutsches QWERTZ-Layout ableiten.

## Vial und Persistenz

UID, Unlock-Kombination `[0,0]` + `[0,1]`, Geometrie und 16 dynamische Layer
entsprechen dem Hersteller. Zum Entsperren physisch SYS/Grave und Q halten.
VialRGB, Tap Dance, Combos, Key Overrides und Repeat Key bleiben verfügbar.

Die eigenen Voreinstellungen sind 250 ms, Permissive Hold und Chordal Hold an,
Hold On Other Key Press aus. Sie gelten global für normale MT/LT-Tasten.
Die explizite Handzuordnung deckt alle 46 Positionen ab; die sechs Daumen sind
mit `*` ausgenommen. QMKs automatische Tabelle würde im verwendeten Branch nur
das erste Herstellerlayout mit 36 Positionen berücksichtigen.

`TD(0)` enthält Cmd+Space/Hyper, `TD(1)` Caps Word/SHORT, `TD(2)` Komma/Punkt.
Alle drei starten mit 250 ms, sind aber einzeln in Vial änderbar. Ihre Entscheidungs-
logik ist unabhängig von Permissive-/Chordal-Hold: Ein kurzer Tap wartet auf
Timeout oder Unterbrechung; eine früh dazukommende Taste entscheidet beim ersten
Tap für Tap statt Hold. SHORT und Hyper erst nach Ablauf der Hold-Frist verwenden.
Leere Doppeltipp-/Tap-Hold-Felder verwenden Vials eingebaute Fallbacks.

Die einzige eigene Laufzeitlogik ist die Initialisierung: `via_init_kb()` merkt
sich Vials Gültigkeitszustand. `keyboard_post_init_user()` setzt auf dem USB-Master
bei ungültigem Speicher oder abweichender User-Versionskennung die QMK-Settings
(Protokoll-IDs 7/22/23/26) und die drei Tap Dances. Erst danach wird die Kennung
abgelegt. Normale Neustarts erhalten GUI-Änderungen. Ein explizites Zurücksetzen
in Vial kann dagegen Vials eigene Resetwerte herstellen.

Der Branch verwendet eine zufällige Build-ID als Gültigkeitskennung für die
persistente Belegung. Ein neues Image kann diese neu initialisieren. Änderungen
an Layeranzahl oder Features können ebenfalls das Speicherlayout ändern.
Vor späteren Firmwarewechseln Vial-Einstellungen exportieren. Es gibt keinen
eigenen pauschalen EEPROM-Reset-Hook.

Caps Word verwendet den Standard: fünf Sekunden Idle-Timeout, Großschreibung
von Buchstaben und Minus zu Unterstrich, Vials Shift-Invertierung aktiv.
RGB-Startwerte, Helligkeitsgrenze, Effektauswahl und Suspendverhalten bleiben beim
Herstellerstandard. Assistant (`KC_ASST`) und System Power (`KC_PWR`) sind normale
QMK-Codes; ihre Wirkung auf dem Mac ist noch nicht bestätigt.

SYS enthält unten außen nach innen auf beiden Hälften `QK_RBT`, `QK_BOOT`, `EE_CLR`.
`EE_CLR` verwendet den normalen sofortigen QMK-Handler ohne eigene Haltedauer.
Diese Aktionen laufen auf dem USB-Master; die Position auf der rechten Hälfte
adressiert nicht automatisch den rechten Controller. Es gibt keine eigene Split-RPC.

## Build und Hardwaretests

Der Workflow baut die unveränderte Herstellerreferenz und die persönliche Keymap
in einem gemeinsamen Aufruf und veröffentlicht getrennte Artefakte. Vorlage ist
[Keebarts CI am festgesetzten Commit](https://github.com/Keebart/vial-qmk-keebart/blob/63e2d6248209f1926e60daa5b950c771cf09d837/.github/workflows/ci.yml):
gleicher Container, direkte vorinstallierte Umgebung, Git-Safe-Directory und
`qmk mass-compile -j "$(nproc)"`. Statt aller Herstellerboards werden nur unsere
beiden Targets mit einem gemeinsamen Budget paralleler Jobs gebaut. Erfolgreich
erzeugte UF2-Dateien werden auch bei einem Teilfehler hochgeladen; der Lauf bleibt
dann fehlgeschlagen. Bei Abbruch entfallen die UF2-Uploads. Fehlerlogs erscheinen
bei Fehlschlag im Joblog.
Zusätzlich braucht External Userspace zwei Checkouts, die CLI-Konfiguration von
`user.qmk_home`/`user.overlay_dir` und `QMK_USERSPACE`; Prüfungen und Artefakt-
Uploads bleiben projektspezifisch. Die zusätzliche
venv-Erstellung wurde nach dem gemeldeten `ensurepip`-Fehler entfernt.
Firmware-Commit und Container sind festgesetzt; Actions-Tags und zufällige Build-ID
verhindern eine Zusage bitidentischer Builds. Die Artefakte enthalten Build-Provenienz.

`check_sources.py` prüft Herstellerstand, Referenzdateien und Geometrie.
`tests/run.py` prüft die kompilierten Keymap-Daten und 32 Ereignisszenarien mit
echten Keebart-Tap/Hold-, Tap-Dance- und Caps-Word-Implementierungen; Hardware,
USB und EEPROM sind simuliert. Ein vollständiger ARM-/UF2-Build ist damit nicht
bestätigt. Ein erfolgreicher Online-Build nach der Workflowkorrektur und
Hardwaretests stehen aus.

Vor dem Einsatz noch zu prüfen:

1. Beide vollständigen UF2-Builds in Actions und ihre Build-Provenienz.
2. Eigenständiger physischer Bootloaderzugang beider Hälften sowie USB links/rechts.
3. Seitenerkennung, Schalter, RGB und gegebenenfalls Encoder; vier innere Tasten belegen.
4. Alle Layer, HRM-Rollen/Akkorde, Quick Tap und die drei Tap Dances einschließlich
   Unterbrechung, Doppeltipp und Host-Wiederholung; Assistant und Power auf dem Mac.
5. Vial-Entsperren, 16 Layer, Einstellungsänderungen, Export und Erhalt nach Neustart.

Nichts automatisch flashen oder zurücksetzen. Ein tatsächliches EEPROM-Löschen
ist kein Teil dieser Prüfungen. TRRS nur im stromlosen Zustand umstecken.
