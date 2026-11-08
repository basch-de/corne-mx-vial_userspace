# Corne MX Pro – QMK/Vial External Userspace

Persönliche Firmware für die **Keebart Corne MX Pro Standard, RP2040, 46 Tasten,
keine Displays**. Sie verwendet Keebarts normale QMK-/Vial-Verarbeitung.
Die Belegung basiert ursprünglich auf der Corne-Choc-ZMK-Konfiguration und wird
jetzt direkt in [keymap.c](keyboards/keebart/corne_mx_pro/keymaps/basch_de/keymap.c)
gepflegt. Es gibt keinen Belegungsgenerator und keine Abhängigkeit vom ZMK-Repository.

| Keymap | Zweck |
|---|---|
| `basch_de` | Persönliche Belegung: acht vorbelegte von 16 dynamischen Layern |
| `vial` | Direkt aus dem Firmwarecheckout: vier vorbelegte von 16 Layern |

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
`vial`. Jeder Lauf ermittelt dessen aktuellen Commit einmal und verwendet ihn
anschließend für beide Buildvarianten. Neue Keebart-Commits werden beim nächsten
Push, Pull Request oder manuellen Start übernommen; sie starten allein keinen Lauf.
Die tatsächliche Firmware-Revision steht in den Buildnachweisen.
Er verwendet External Userspace, einen festgesetzten QMK-Buildcontainer und
rekursive Submodule. Container und `qmk mass-compile`-Buildaufruf sind aus
[Keebarts CI](https://github.com/Keebart/vial-qmk-keebart/blob/63e2d6248209f1926e60daa5b950c771cf09d837/.github/workflows/ci.yml)
übernommen und auf unsere beiden Targets begrenzt. Userspace-Pfade, Prüfungen
und Artefakt-Uploads ergänzen den Herstellerablauf. Die vorinstallierte
Buildumgebung wird direkt verwendet; eine zusätzliche Python-venv wird nicht angelegt.
Hardwaredefinitionen und QMK-Core werden nicht verändert.

Nach einem Push oder einem manuellen Workflowstart baut Actions die
Herstellerreferenz und die persönliche Keymap in zwei unabhängigen Matrix-Jobs.
Der Herstellerjob verwendet einen separaten Userspace mit lediglich `qmk.json`
und baut Keebarts originale `vial`-Keymap direkt aus dem Firmwarecheckout.
Lokal liegt nur die persönliche Keymap `basch_de`; die Herstellerkopie entfällt.
Persönliche Prüfungs- oder Buildfehler verhindern die Hersteller-UF2 nicht.
Jeder Job prüft ausdrücklich, ob seine UF2 erzeugt wurde, und lädt sie als
separates Artefakt hoch. Scheitert ein Job, bleibt der gesamte Lauf fehlgeschlagen.
Revisionen, Werkzeugversionen, Compilerlogs, Fehlerlogs und UF2-Prüfsummen werden
je Variante unter `build-provenance-manufacturer` bzw. `build-provenance-personal`
archiviert; die Nachweise bleiben auch bei Fehlern verfügbar. Es gibt keine Flash-Schritte.
Die UF2-Artefaktnamen bleiben `corne-mx-pro-keebart-vial-stock` und
`corne-mx-pro-basch-de-vial`; die Herstellerdatei endet jetzt auf `_vial.uf2`.

Mit einem vorhandenen Checkout des zu prüfenden Keebart-Firmwarestands:

```sh
python3 scripts/check_sources.py /path/to/vial-qmk-keebart
python3 tests/run.py /path/to/vial-qmk-keebart
git diff --check
```

Die Quellenprüfung akzeptiert neue Commit-IDs und prüft dafür Hardwaregeometrie,
Vial-Identität, 16 Layer, Initialisierungshooks und die verwendeten Settings-Protokoll-IDs.
Relevante Herstelleränderungen können Anpassungen der persönlichen Firmware erfordern.

**Prüfstand:** Lokal geprüft ist Firmware-Commit
`63e2d6248209f1926e60daa5b950c771cf09d837`; das ist kein aktiver Build-Pin.
Herstellerreferenz und Vial-Geometrie geprüft; beide Keymaps auf
dem Host mit echten QMK-Keycodes und Herstellerlayout kompiliert. 35 Host-
Ereignistests sowie Prüfungen der tatsächlichen Keymap-Matrix bestehen. Die Tests
verwenden die kompilierten Tasten ihrer jeweiligen Layer und prüfen unter anderem
Press/Release-Paare, freigegebene Layer und Ausgaben sowie Initialisierung und
Erhalt fremder Einstellungen. Hardware, USB und persistenter Speicher werden dabei
simuliert. Der Nutzer hat einen
erfolgreichen Actions-Build bestätigt; Lauf-ID, gebaute Revisionen und Artefaktinhalte
wurden dazu nicht separat geprüft. Hardware- und Vial-GUI-Tests stehen noch aus.
Die zusätzliche venv-Erstellung, die zuvor am fehlenden `ensurepip` scheiterte,
wurde entfernt.
Es wurde nichts geflasht und kein EEPROM-Reset auf der Tastatur ausgeführt.

Technische Hinweise, Persistenz und offene Hardwaretests:
[docs/PROJECT.md](docs/PROJECT.md).
