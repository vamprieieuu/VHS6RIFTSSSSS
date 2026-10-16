# VHS Master — Automated Test Plan & QA Protocol

This document outlines the testing protocol and verification scripts for **VHS Master**.

---

## 1. Static Parameter & Code Integrity Test
Run `python3 tools/validate_project.py` to confirm that:
- All 112 parameter IDs in `include/VHS6RIFT.h` match the parameter setup registration sequence in `src/VHS6RIFT.cpp`.
- Every parameter enum is referenced in render processing logic.
- PiPL resource definitions, entry point exports, and version headers (`PF_VERSION(1, 2, 0, PF_Stage_RELEASE, 1)`) match across files.

---

## 2. Functional & Algorithmic Verification
Run `python3 tools/test_vhs6rift.py` to test:
- **Master Strength:** Verifies bit-exact identity bypass at strength = 0 (zero black screen).
- **Camera Profiles:** Verifies that all 7 camera profiles produce mutually distinct color and contrast signatures.
- **Glitch Scheduler:** Verifies time-deterministic glitch event scheduling across timeline playback.
- **Audio Glitch DSP:** Verifies sample-domain `tanh` saturation and bit-depth quantization.

---

## 3. Visual Validation Preview Generation
Run `python3 tools/generate_previews.py` to regenerate:
- `previews/original.png`
- `previews/camera_fxc6.png`, `camera_gen7.png`, `camera_jvc_c7u.png`
- `previews/vhs_sp_mode.png`, `vhs_lp_mode.png`
- `previews/bad_signal_glitch.png`
- `previews/crt_tv_trinitron.png`
- `previews/grain_color.png`
- `previews/date_text_osd.png`
- `previews/vhs_master_full_suite.png`
- `previews/VISUAL_COMPARISON.png`

---

## 4. After Effects Runtime Installation & QA (Windows 11)
1. Build `VHS6RIFT.aex` using `build.ps1` or CMake with MSVC 2022.
2. Copy `VHS6RIFT.aex` to `C:\Program Files\Adobe\Adobe After Effects 2023\Support Files\Plug-ins\Effects\`.
3. Open After Effects 23.2.1, create a 1920x1080 composition at 29.97fps, and apply **Effects -> Video Effects -> VHS Master**.
4. Test parameter tweaking in the Effect Controls panel to confirm smooth real-time rendering.
