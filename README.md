# VHS Master

**Native Adobe After Effects Effect Plugin — After Effects 23.2.1 / Windows 11 / x64 / Visual Studio 2022.**

**VHS Master** is a native Adobe After Effects effect plugin written in C++ that recreates vintage analog video, CRT display, camcorder OSD typography, bad signal glitch artifacts, and sample-domain audio degradation.

---

## Target Environment

* **Host:** Adobe After Effects 2023 (v23.2.1)
* **OS:** Windows 11 x64
* **Compiler:** Visual Studio 2022 (MSVC x64, C++17)
* **SDK:** Adobe After Effects SDK (located under `extern/AfterEffectsSDK/`)
* **Plugin Binary:** `VHS6RIFT.aex` (or `VHS_Master.aex`)

---

## Effect UI Groups & Controls

VHS Master organizes controls into logical, compact parameter sections:

### 1. Camera Engines
Profiles derived from reference camcorders and broadcast relays:
* **Profiles:** Fxc6 Reference, Gen 7 Reference, JVC GR-C7U (1986), VHS-R20 Reference, Fall-78 Reference, ERC-V2S Reference, JVC GR-SXM740U.
* **Controls:** Exposure Shift, Highlight Rolloff, Shadow Lift, Color Saturation, WB Temp/Tint, Optical Softness, Sharpness, Luma/Chroma Bandwidth, Chroma Bleed, Lowlight Noise, Format Response Drive.

### 2. VHS Degradation Engine
* **Modes:** SP Standard, LP Long Play, EP Extended Play, Hi-Fi Master.
* **Controls:** Tape Strength, Tape Band Size, Transport Speed, Phase Jitter, Dropout Density, Wrinkle Distortion, Chroma Deformation.

### 3. Bad Signal / Glitch Engine
* **Controls:** Enable Glitch Engine, Glitch Frequency, Duration, Intensity, Pattern Variation, Recovery Speed, Horizontal Tearing, Band Disruption, Tracking Fail Rate, Chroma Instability, Burst Flicker, Glitch Speed, Seed.

### 4. CRT TV Engine
* **Styles:** Trinitron CRT, Consumer TV, JVC Studio Monitor.
* **Controls:** Scanlines, Phosphor Glow / Bloom, Screen Curvature, RGB Convergence Shift, CRT Flicker & Vignette.

### 5. Grain Engine
* **Modes:** Monochrome, Color Organic, RF Hiss Noise.
* **Controls:** Enable Grain, Amount, Luma/Chroma Grain Strength, Grain Size/Softness, Shadow/Midtone/Highlight Noise Response, Motion Speed, Seed.

### 6. Dates & Text OSD
* **Styles:** Date & Time, PLAY + Date, REC + Timestamp, AUTO SP, Custom Text Preset.
* **Controls:** Enable OSD, Year (1970–2030), Month, Day, Hour, Minute, Preset String (PLAY, REC ●, PAUSE, AUTO SP, STOP, VHS MASTER), Position X/Y, Scale, Opacity.

### 7. Advanced Overlays & Audio
* **Controls:** Layer Overlays (Transparent Tape, Moving Sweep, Oxide Dust, Glitch Bars), Blend Modes, Audio Glitch Processing (Saturation, Lo-Fi Quantization, Static, Dropouts, Flutter).

---

## Building the Plugin

### SDK Directory Setup
Ensure the Adobe After Effects SDK is present in `extern/AfterEffectsSDK/`:
```
extern/AfterEffectsSDK/Headers/AE_Effect.h
extern/AfterEffectsSDK/Resources/PiPLtool.exe
```

### PowerShell Build Command
Run `build.ps1` in PowerShell:
```powershell
.\build.ps1
```

### Manual CMake Build (Visual Studio 2022 MSVC x64)
```cmd
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DAE_SDK_ROOT="extern/AfterEffectsSDK"
cmake --build build --config Release --parallel
```
The compiled output will be generated under `build/Release/VHS6RIFT.aex`.

### Installing into After Effects
Copy `VHS6RIFT.aex` to your After Effects Plug-ins folder:
`C:\Program Files\Adobe\Adobe After Effects 2023\Support Files\Plug-ins\Effects\`

---

## Visual Previews & Validation
Rendered preview outputs demonstrate the image-processing behavior:
* `previews/original.png` — Original test footage
* `previews/camera_fxc6.png` — Fxc6 profile output
* `previews/camera_gen7.png` — Gen 7 profile output
* `previews/camera_jvc_c7u.png` — JVC GR-C7U profile output
* `previews/vhs_sp_mode.png` — VHS SP mode output
* `previews/bad_signal_glitch.png` — Bad Signal / Glitch output
* `previews/crt_tv_trinitron.png` — CRT TV output
* `previews/grain_color.png` — Organic grain output
* `previews/date_text_osd.png` — Date/Text OSD typography
* `previews/vhs_master_full_suite.png` — Combined full effect render
* `previews/VISUAL_COMPARISON.png` — Full multi-mode comparison grid

---

## License & Credits
VHS Master source code is developed specifically for Adobe After Effects native effect plugin development.
