# Projekt und Arbeitsbereich

QMK External Userspace für die **Keebart Corne MX Pro Standard, kabelgebunden,
zwei RP2040, 46 physische Tasten, keine Displays**.

- Buildziel: `keebart/corne_mx_pro/standard`
- Eigene Keymap: `basch_de`
- Unveränderte Herstellerreferenz: `vial`, direkt aus dem Firmwarecheckout
- Kommunikation auf Deutsch; Code, Bezeichner und Quelltextkommentare auf Englisch.
- Vor Änderungen tatsächlichen Dateistand und lokale Änderungen prüfen.

## Verbindliche Leitlinie

Die Belegung stammt ursprünglich aus der Corne-Choc-ZMK-Konfiguration. Der Nutzer
hat ausdrücklich normale Keebart-/Vial-Verarbeitung statt möglichst genauer
ZMK-Verhaltensnachbildung gewählt. Die QMK-Dateien werden jetzt eigenständig gepflegt.

- `keymap.c` ist die maßgebliche, direkt zu bearbeitende Belegung. Es gibt keinen
  Belegungsgenerator, keine separate Bindungs-JSON und keine Prüfabhängigkeit vom
  benachbarten ZMK-Repository. Keine automatische Rückübertragung aus ZMK einführen.
- Standardimplementierungen für Mod-/Layer-Taps, Tap Dance, Caps Word, RGB und `EE_CLR` nutzen.
- Keine eigenen Timingentscheidungen, ersetzten Tap-Ausgaben, Caps-Word-Overrides,
  Reset-Timer oder Compiler-Symbolumbenennungen ohne neue Anforderung einführen.
- Eigene Laufzeitlogik beschränkt sich auf die Initialisierung der Vial-Settings
  und Tap Dances. Die Handzuordnung ist Konfiguration über den vorgesehenen QMK-Mechanismus.
- Belegung nicht pauschal neu entwickeln. Nur beauftragte Änderungen vornehmen.
- Normale Neustarts dürfen gespeicherte Vial-Änderungen nicht überschreiben.

## Hardware

Die MX hat 46 nutzbare Tasten und keine Displays. Nicht mit der früheren Corne
Choc Pro BT verwechseln. Das Layout `LAYOUT_split_3x6_3_ex2` hat 14 + 14 + 12 + 6
Argumente. Vier zusätzliche innere MX-Tasten sind bisher auf allen Layern unbelegt:

| Layoutposition (ab 0) | Matrix | Lage |
|---|---|---|
| 6 | `[0,6]` | links innen oben |
| 7 | `[4,6]` | rechts innen oben |
| 20 | `[1,6]` | links innen Mitte |
| 21 | `[5,6]` | rechts innen Mitte |

Die Belegung dieser vier Positionen ist noch offen; `KC_NO` ist keine Hardware-
Einschränkung. Bei Zuweisung die entsprechenden Keymap-Prüfungen in `tests/sequences.c`
anpassen. Rechte Matrixspalten sind gespiegelt; Daumen links `[3,3..5]`, rechts
in Layoutreihenfolge `[7,5]`, `[7,4]`, `[7,3]`.

Herstellerdefinition bestätigt: `SPLIT_HAND_PIN GP21`, Split-TX GP12, USB-VBUS GP13.
Hardwaredefinitionen unverändert aus Keebarts Firmware beziehen.

Die explizite `chordal_hold_layout` umfasst alle 46 Positionen; sechs Daumen sind
mit `*` ausgenommen. Sie ist beabsichtigt: QMK erzeugt im bisher geprüften Stand
seine automatische Handtabelle nur aus dem ersten Herstellerlayout mit 36 Tasten.
Daumenausnahmen sind unsere Belegungsentscheidung, kein universeller Standard.

## Quellen und Architektur

- Firmware: <https://github.com/Keebart/vial-qmk-keebart>, Branch `vial`.
  Jeder neue Workflow-Lauf löst den aktuellen Branchstand einmal im Job
  `firmware-revision` auf. Beide Buildvarianten verwenden danach dieselbe Commit-ID.
  Keinen dauerhaften Firmware-Commit-Pin wieder einführen. Neue Hersteller-Commits
  starten allein keinen Lauf; Trigger sind Push, Pull Request und manueller Start.
  `63e2d6248209f1926e60daa5b950c771cf09d837` ist der bisher lokal geprüfte Stand,
  kein aktiver Checkout-Pin und kein Nachweis für neuere Revisionen.
- Historische Belegungsvorlage: <https://github.com/basch-de/zmk-config>, Commit
  `17dc18d4065161066a459656a87f36f11cd4ae88`, `config/corne_choc_pro.keymap`.
  Für normale QMK-Arbeit muss diese Quelle nicht erneut ausgewertet werden.
- `../qmk_userspace` ist lediglich Strukturreferenz für eine Preonic.
  Das alte `../qmk_firmware` ist keine fachliche Referenz für dieses Projekt.

External Userspace beibehalten. Keebarts untersuchter Branch unterstützt
`QMK_USERSPACE` nativ; keine Keymapkopien in den Firmwarebaum und keine Core-
oder Hardwarepatches nötig. Versionsabhängige APIs gegen den Buildstand prüfen.
Keebart Studio und dessen Feature-Branch werden im separaten Projekt
`../corne-mx-studio_userspace` gepflegt. Es dient hier als lesbare Buildreferenz;
Änderungen daran sind ein eigener Auftrag.

## Dateien pflegen

Eigene Keymap-Dateien liegen unter `keyboards/keebart/corne_mx_pro/keymaps/basch_de/`:

| Datei | Aufgabe |
|---|---|
| `keymap.c` | Direkt gepflegte Layer- und Encoderbelegung |
| `behaviors.h` | Layernamen und Aliase für normale MT/LT-Codes |
| `behaviors.c` | Erstinitialisierung und Chordal-Hold-Handzuordnung |
| `config.h` | Anfänglicher Tapping Term und Hersteller-Vial-Identität |
| `rules.mk` | Standardfeatures und zusätzliche Quelldatei |
| `vial.json` | Unveränderte Herstellergeometrie |

Keine lokale Kopie der Hersteller-Keymap pflegen. Der Herstellerjob baut `vial`
direkt aus dem für den Lauf aufgelösten Firmwarecheckout. Er verwendet `stock-userspace/`
mit lediglich einer kopierten `qmk.json`, ohne lokale Keymap-Dateien.
Der persönliche Job verwendet `userspace/` mit der lokalen Keymap `basch_de`.
`qmk.json` enthält beide Buildziele. Der Workflow baut sie in zwei unabhängigen
Matrix-Jobs (`manufacturer` und `personal`) mit `fail-fast: false` und je einem
`qmk mass-compile -j "$(nproc)"`-Aufruf. Persönliche Quellen- und Hosttests laufen
nur im persönlichen Job. Die Quellenprüfung protokolliert den Hersteller-Commit
und prüft Hardware- und Vial-Geometrie, 16 Layer, UID, Unlock und die vier verwendeten
Settings-Protokoll-IDs gegen den Firmwarecheckout. Änderungen können einen neuen
Audit oder Anpassungen erfordern; Prüfungen nicht zur Fehlerumgehung entfernen.
Eine fehlende erwartete UF2 muss den Job fehlschlagen lassen. Erfolgreiche Jobs
laden ihre UF2 auch dann hoch, wenn der andere Job scheitert. Compilerlogs,
Fehlerlogs, Revisionen und UF2-SHA256 je Variante als Build-Provenienz archivieren;
Provenienz auch bei Fehlern hochladen. Keine UF2-Uploads nach einem fehlgeschlagenen
oder abgebrochenen eigenen Build starten.
Buildvorlage ist Keebarts `.github/workflows/ci.yml` am oben genannten Analysestand:
Container, direkte Umgebung, Git-Safe-Directory und `qmk mass-compile` verwenden.
Für External Userspace kommen zweiter Checkout, `QMK_USERSPACE`, CLI-Pfade,
unsere Prüfungen und Artefakt-Uploads hinzu. Keine zusätzliche venv einführen.
Der Buildcontainer bleibt per Digest fest; Actions bleiben auf `@v4`, Runner auf
`ubuntu-latest`. Submodule rekursiv an den Commits der aufgelösten Firmware laden,
nicht unabhängig auf die neuesten Submodule-Branches wechseln.
`README.md` ist der Einstieg; [docs/PROJECT.md](docs/PROJECT.md) enthält technische
Hinweise und offene Hardwaretests. Diese Dokumentation bei relevanten Änderungen aktualisieren.

Alle Layer in der physischen Vier-Reihen-Struktur ausrichten: 14, 14, 12 und
sechs Argumente, vierzehn logische Spalten. Vollständige Keycode-Ausdrücke bleiben
ungeteilte Zellen. Keine Belegungsänderungen nur zum Formatieren vornehmen.

## Belegung und Einstellungen

Layer: `MACOS=0`, `NUM=1`, `SYM=2`, `NAV=3`, `WIN=4`, `SHORT=5`, `NUMPAD=6`, `SYS=7`.

- Backspace halten aktiviert NUM; Tab halten aktiviert WIN.
- WIN enthält Ctrl+Alt-Fensteraktionen, kein Windows-Basislayout.
- NUMPAD verwendet normale Zahlen; beide Toggle-Wege erhalten.
- US-HID-Codes und Modifier sind maßgeblich. Aus `basch_de` kein deutsches QWERTZ ableiten.
- 16 dynamische Layer sind Herstellerstandard; acht davon vorbelegt. Keine Reduzierung auf acht.
- Encoder-Map und Keymap müssen gleich viele vorbelegte Layer besitzen.
- Die persönliche `vial.json` entspricht der Herstellerdefinition ohne Custom Keycodes.

Vial-Settings: `QMK_SETTINGS = yes`, Tapping Term 250 ms, Permissive Hold und
Chordal Hold an, Hold On Other Key Press aus. Quick Tap startet nach Vials
Standard mit 250 ms ab Loslassen, Flow Tap mit 0. Diese Regeln gelten global
für normale MT/LT-Tasten; die alten ZMK-Flavors werden nicht nachgebildet.
250 ms ist Pascals HRM-Ausgangspunkt. QMKs Standard ohne Override wäre 200 ms.

| Tap Dance | Tap | Hold | Anfangsfrist |
|---|---|---|---|
| `TD(0)` | `LGUI(KC_SPC)` | `HYPR(KC_NO)` | 250 ms |
| `TD(1)` | `CW_TOGG` | `MO(SHORT)` | 250 ms |
| `TD(2)` | `KC_COMM` | `KC_DOT` | 250 ms |

Die drei Fristen verwenden derzeit ebenfalls `TAPPING_TERM`; dies ist keine
zusätzliche Empfehlung von Pascal. Vial speichert jede Frist separat und ändert
sie bei globalen Timingänderungen nicht automatisch. Doppeltipp-/Tap-Hold-Felder
sind 0 und verwenden Vials Standardfallbacks. Keine eigenen Tap-Dance-Callbacks
oder parallelen `tap_dance_actions` definieren. Frühe Unterbrechung entscheidet
beim ersten Tap für Tap; für SHORT/Hyper muss die Hold-Frist ablaufen.

`KC_ASST` (Consumer AL Assistant 0x1CB) und `KC_PWR` (System Power 0x81) sind
bewusst andere HID-Aktionen als die ehemaligen ZMK-Consumer-Codes. Hostwirkung prüfen.
Caps Word verwendet den Standard: fünf Sekunden Idle-Timeout, auch Minus wird zu
Unterstrich, Vials Shift-Invertierung aktiv. Keine eigenen Overrides.

## Initialisierung und Persistenz

`via_init_kb()` merkt sich die Vial-Speichergültigkeit vor der Coreinitialisierung.
`keyboard_post_init_user()` setzt nur auf dem USB-Master und nur bei ungültigem
Vial-Speicher oder abweichender User-Versionskennung unsere vier Settings über
`qmk_settings_set()` und drei Einträge über `dynamic_keymap_set_tap_dance()`.
Die Kennung wird erst nach erfolgreichen Schreibvorgängen gespeichert.
`eeconfig_init_user()` leert sie, wenn QMK selbst seine Einstellungen initialisiert.
Die Protokoll-IDs 7/22/23/26 bei Firmwareupdates gegen `qmk_settings.c` prüfen.

Normale Neustarts erhalten GUI-Werte. Explizites Rücksetzen über Vial kann dessen
eigene Resetwerte herstellen. Die zufällige Vial-Build-ID kann nach einem Image-
wechsel eine Neuinitialisierung auslösen; auch Feature-/Layeränderungen beeinflussen
das Speicherlayout. Keine zusätzlichen pauschalen EEPROM-Reset-Hooks einbauen.

## Firmware-Sicherheit und SYS

Ausdrückliche Nutzeranweisungen:

- Keine Firmware automatisch flashen.
- Keine EEPROM-Resets oder vollständigen Flash-Löschungen ausführen.
- Zunächst nur Quellcode und Buildsystem; UF2-Dateien über Actions bereitstellen.

SYS enthält unten auf beiden Hälften außen nach innen `QK_RBT`, `QK_BOOT`, `EE_CLR`.
`EE_CLR` verwendet auf Nutzerauftrag den normalen sofortigen Handler, ohne
Haltedauer, eigenen Record-/Layer-State-Hook oder Bestätigung. Zugang erfolgt über SYS.
Keymap-Aktionen laufen auf dem USB-Master; rechts platzierte Reset-/Bootloader-
Tasten adressieren nicht automatisch den rechten Controller. Keine eigene Split-RPC.
Beide physischen Bootloaderzugänge später unabhängig prüfen.

## Prüfungen und Status

```sh
python3 scripts/check_sources.py /path/to/vial-qmk-keebart
python3 tests/run.py /path/to/vial-qmk-keebart
git diff --check
```

`check_sources.py` prüft Kompatibilität statt einer festen Firmware-Commit-ID:
Hardware- und Vial-Geometrie, Identität, Layerzahl und Settings-Protokoll-IDs.
Die Host-Kompilierungsprüfung bezieht die originale `vial`-Keymap direkt aus dem
Firmwarecheckout; nur die persönliche Keymap liegt lokal.
Die Hosttests prüfen die tatsächlichen kompilierten Keymap-Daten und 32 Ereignis-
szenarien mit unveränderten Keebart-Tap/Hold-, Tap-Dance- und Caps-Word-Funktionen.
Hardware, USB, EEPROM und Reportgrenzen sind simuliert. Nach Belegungsänderungen
betroffene Matrixprüfungen anpassen; keine separate Belegungskopie pflegen.
Auch neue unversionierte Dateien auf Whitespacefehler prüfen. Für reine
Dokumentationsänderungen sind keine Firmwaretests erforderlich.

Stand: Herstellerreferenz und Keymaps auf dem Host geprüft. Der Nutzer hat einen
erfolgreichen Actions-Build gemeldet. Lauf-ID, gebaute Firmware-/Userspace-Revision
und Artefaktinhalte wurden dazu nicht separat geprüft; den Erfolg nicht pauschal
auf spätere Revisionen übertragen. Der Workflow verwendet direkt die vorinstallierte
Umgebung des festgesetzten Containers. Hardware-/GUI-Tests und vier
zusätzliche Tastenbelegungen bleiben offen. Kein Flash oder EEPROM-Reset ausgeführt.
Der Firmware-Commit wird pro Lauf gemeinsam aufgelöst und protokolliert; der
Buildcontainer bleibt festgesetzt. Branchupdates, Actions-Tags und zufällige
Build-ID erlauben keine Zusage bitidentischer Builds. Der temporäre Analysecheckout
unter `/private/tmp/corne-keebart-vial` folgt dem Branch nicht automatisch.

Erfolgreichen Hosttest nicht als erfolgreichen Firmwarebuild melden. Build- und
Hardwareergebnisse nur für den konkret geprüften Stand berichten und dokumentieren.
