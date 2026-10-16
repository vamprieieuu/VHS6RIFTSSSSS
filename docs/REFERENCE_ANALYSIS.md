# VHS Master — Comprehensive Reference Analysis

This document details the visual, temporal, and signal analysis of all reference media files in `references/VHS MASTER/`.

---

## 1. Inventory & Reference Scope

The reference repository contains 74 media assets divided into 6 main subfolders:
1. `Vhs cams_`: Footage clips recorded on or emulating specific vintage camcorders (Fxc6, Gen 7, JVC GR-C7U, JVC GR-SXM740U).
2. `glitch cam`: Glitch GIFs and JPEGs demonstrating sudden signal disruptions, horizontal tearing, and chroma bursts.
3. `crt tvs`: CRT monitor animated GIFs and videos showing scanlines, screen curvature, phosphor glow, and flicker.
4. `retro vhs`: Classic VHS tape degradation samples showing tracking bands, oxide dropouts, and Y/C separation.
5. `text`: OSD typography references showing date, time, and status text (`PLAY`, `REC`, `AUTO SP`).
6. `overlays as effects`: Models 1 through 12+ overlay video demonstrations.

---

## 2. Signal & Artifact Analysis

### Analog Color Degradation & Bandwidth Limiting
* **Luma Softness:** Analog VHS records luminance with approximately 240 TV lines of horizontal resolution (~3 MHz bandwidth). This produces edge softness along high-contrast transitions.
* **Chroma Bleed:** Chrominance is recorded on a 629 kHz color-under carrier with significantly lower bandwidth (~0.4 MHz), causing color signals (especially saturated reds) to smear horizontally by 4–12 pixels beyond luminance boundaries.
* **Format Response:** Higher-end Super VHS (S-VHS) and Betacam formats preserve higher luma bandwidth while consumer VHS SP/LP/EP modes exhibit progressive softness, noise, and phase jitter.

### Bad Signal & Glitch Breakdown
* **Horizontal Tearing:** Caused by loss of horizontal sync lock, displacing video lines horizontally across localized vertical bands.
* **Chroma Instability & Bursts:** Temporary phase drift in the 3.58 MHz color subcarrier, causing color channel shifts (e.g., strong magenta/green fringing).
* **Block & Line Corruption:** Magnetic oxide erosion or digital signal dropout causing dark or white horizontal streaks.

### CRT Monitor Aesthetics
* **Scanline Structure:** Interlaced raster lines with alternating dark line gaps.
* **Phosphor Bloom:** High-luminance pixels illuminate neighboring phosphor triads, creating a soft highlight glow shoulder.
* **Screen Curvature:** Spherical geometry distortion caused by curved glass CRTs.
* **RGB Convergence:** Misalignment of electron guns producing red and blue fringing along high-contrast vertical edges.

---

## 3. Algorithm Mapping Matrix

| Observed Artifact | Physical Cause | Implementation in VHS Master |
| --- | --- | --- |
| Red/Cyan Chroma Bleed | 629 kHz Color-Under Carrier | Horizontal YUV chroma offset & low-pass blur in `Render()` |
| Optical & Tape Softness | Lens / Luma Bandwidth Limit | Spatial 3x3 low-pass box filter controlled by `camSoftness` and `camLumaBW` |
| Edge Ringing & Sharpness | FM Transmit Peaking | 3x3 unsharp mask acutance filter controlled by `camSharpness` |
| Horizontal Tearing | Horizontal Sync Loss | Deterministic row displacement driven by `glitchTearing` & `glitchEnv` |
| CRT Scanlines & Bloom | Interlaced Raster & Phosphor | Cosine scanline modulation & highlight halo shoulder in `Render()` |
| Camcorder OSD Typography | On-Screen Display Generator | 5x7 ASCII bitmap font rendering in `DrawOSDText()` |
