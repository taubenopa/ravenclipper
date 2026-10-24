# RavenClipper

Kostenloser **VST3 Soft Clipper für Beats** (Windows + macOS) mit animierten Raben im Interface.
Gedacht für Drums, 808s und die Beat-Summe: knackige Transienten, kein hartes Zerren.

## Was drin ist

| Regler | Funktion |
|---|---|
| **Drive** | Wie stark das Signal in den Clipper gedrückt wird (0–24 dB). |
| **Edge** | Härte des Knies. Links: Kurve greift früh und sanft ein. Rechts: Signal bleibt lange unangetastet, Clipping setzt spät, aber enger ein. |
| **Ceiling** | Harte Obergrenze am Ausgang (-12 bis 0 dB). Der Clipper kann nie darüber hinaus (siehe `tests/dsp_test.cpp`). |
| **Mix** | Parallel-Clipping. 40–70 % erhält den Punch von Kicks und Snares. |
| **Output** | Ausgangspegel nach dem Clipper. |
| **Shape** | *Tanh* (glatt), *Cubic* (rund), *Sine* (musikalisch), *Warm* (asymmetrisch, gerade Harmonische). |
| **Oversampling** | Off / 2x / 4x / 8x (Linear-Phase-FIR). Verhindert Aliasing. 4x ist der Standard. Die Latenz wird an die DAW gemeldet. |
| **Auto Gain** | Gleicht den Drive wieder aus, damit du *fair* A/B vergleichen kannst und nicht nur "lauter = besser" hörst. |

Dazu: Live-Übertragungskurve, Input/Output-Pegel, und die Raben fliegen schneller, je stärker geclippt wird.

**Startpunkte für Beats**
- *Beat-Summe:* Drive 3–6 dB, Edge 40 %, Shape Tanh, Ceiling -0.3 dB, 4x.
- *808 / Bass:* Shape Warm, Drive 6–10 dB, Edge 25 %, Mix 100 %.
- *Drums:* Shape Sine, Drive 4–8 dB, Edge 55 %, Mix 60 %.

## Auf GitHub bringen

1. Auf github.com ein **neues öffentliches Repository** `RavenClipper` anlegen (ohne README).
2. Diesen Ordner entpacken und im Terminal:
   ```bash
   git init -b main
   git add .
   git commit -m "RavenClipper 1.0"
   git remote add origin https://github.com/DEIN-NAME/RavenClipper.git
   git push -u origin main
   ```
3. Tab **Actions**: GitHub baut jetzt automatisch Windows und macOS. Die fertigen Installer findest du danach beim Workflow-Lauf unter *Artifacts*.
4. **Release veröffentlichen** (Installer als Download für alle):
   ```bash
   git tag v1.0.0
   git push origin v1.0.0
   ```
   Danach liegen `RavenClipper-1.0.0-Windows-Setup.exe` und `RavenClipper-1.0.0-macOS.pkg` unter *Releases*.
5. **Lizenzdatei:** Auf GitHub *Add file > Create new file*, Name `LICENSE`, rechts *Choose a license template* > **GNU Affero General Public License v3.0**.
   (JUCE ist AGPLv3-lizenziert. Für ein kostenloses Open-Source-Plugin ist das genau richtig. Verkaufen oder Quellcode verstecken ginge nur mit kommerzieller JUCE-Lizenz.)

## Lokal bauen

Voraussetzung: CMake 3.22+, Windows: Visual Studio 2022 (C++), macOS: Xcode. JUCE wird beim ersten Konfigurieren automatisch geladen.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target RavenClipper_VST3
```
Das Plugin liegt dann in `build/RavenClipper_artefacts/Release/VST3/`.

DSP-Test ohne JUCE: `g++ -std=c++17 -I Source tests/dsp_test.cpp -o dsp_test && ./dsp_test`

## Installer

- **Windows:** `installer/windows/RavenClipper.iss` (Inno Setup 6), installiert nach `C:\Program Files\Common Files\VST3`. Englisch und Deutsch.
- **macOS:** `installer/mac/build_pkg.sh`, installiert nach `/Library/Audio/Plug-Ins/VST3`. Universal (Apple Silicon + Intel).
- Raben-Grafiken neu erzeugen: `pip install pillow numpy && python tools/make_installer_art.py`

### Warnungen beim Installieren (wichtig!)

Ohne bezahlte Zertifikate zeigen die Betriebssysteme Warnungen. Das ist normal für Free-Plugins:
- **Windows:** SmartScreen "Unbekannter Herausgeber" > *Weitere Informationen > Trotzdem ausführen*.
- **macOS:** Beim ersten Öffnen des .pkg: Rechtsklick > *Öffnen*, oder *Systemeinstellungen > Datenschutz & Sicherheit > Trotzdem öffnen*.
- Wer das vermeiden will: Apple Developer Account (99 $/Jahr) für Signierung + Notarisierung (`APP_SIGN_ID`, `PKG_SIGN_ID` in `build_pkg.sh`), bei Windows ein Code-Signing-Zertifikat.

## Projektstruktur

```
Source/ClipperDSP.h        Clipper-Mathematik (ohne JUCE, getestet)
Source/PluginProcessor.*   Parameter, Oversampling, Signalkette
Source/PluginEditor.*      Oberfläche, Regler, Kurve, Pegel
Source/RavenField.h        Animierte Raben (Vektorgrafik)
installer/                 Windows (Inno Setup) und macOS (.pkg)
.github/workflows/         Automatischer Build + Release
```

*VST ist eine Marke der Steinberg Media Technologies GmbH.*
