# VHS Master — Overlay & Typography Reference Analysis

This document details the inspection and implementation of overlay models and OSD typography references in `references/VHS MASTER/`.

---

## 1. OSD Typography Analysis
* **Source References:**
  - `references/VHS MASTER/new vhs overlays/text/date/`
  - `references/VHS MASTER/new vhs overlays/text/word [play]/`
  - `references/VHS MASTER/new vhs overlays/text/just text/`
* **Visual Characteristics:**
  - Camcorder OSD text uses an un-aliased, mono-spaced bitmap character matrix displayed in pure white (RGB: 255, 255, 255) with optional subtle black drop shadow.
  - Standard displays show status tags in the top-left (`PLAY ▶`, `REC ●`, `AUTO SP`, `PAUSE`) and date/time stamps in the bottom-left (`05/18/1994`, `12:34:00`).
* **Implementation:**
  - Implemented natively via a 5x7 ASCII bitmap matrix array in `src/VHS6RIFT.cpp`.
  - Driven by parameters: `VHS_TEXT_ENABLE`, `VHS_TEXT_STYLE`, `VHS_TEXT_YEAR`, `VHS_TEXT_MONTH`, `VHS_TEXT_DAY`, `VHS_TEXT_HOUR`, `VHS_TEXT_MINUTE`, `VHS_TEXT_PRESET_STRING`, `VHS_TEXT_POS_X`, `VHS_TEXT_POS_Y`, `VHS_TEXT_SCALE`, `VHS_TEXT_OPACITY`.

---

## 2. Damage Overlays & Models 1-12+ Analysis
* **Source References:**
  - `references/VHS MASTER/overlays as effects/More than 12+ modesl for vhs effect .mov`
  - `references/VHS MASTER/overlays as effects/Model 1 .mp4` through `Model 5 .mp4`
  - `references/VHS MASTER/new vhs overlays/retro vhs/`
* **Visual Characteristics:**
  - Overlays fall into four major categories:
    1. Transparent Tape: Horizontal semi-transparent signal abrasion with organic color fringing.
    2. Moving Tape Sweep: Vertical tracking failure sweep moving across the frame.
    3. Oxide Dust: Microscopic magnetic particle specs.
    4. Glitch Bars: Horizontal tearing and disruption bars.
* **Implementation:**
  - Implemented via procedural tape sweep algorithms and layer-based compositing (`PF_CHECKOUT_PARAM` for `VHS_DAMAGE_LAYER`) with Normal, Blend, Add, Screen, Multiply, and Luma Key modes.
