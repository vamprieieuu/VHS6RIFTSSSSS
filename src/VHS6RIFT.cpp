#ifndef _STANDALONE_TEST_HARNESS_
#include "AEConfig.h"
#ifdef AE_OS_WIN
  #include <Windows.h>
#endif
#ifndef DllExport
  #if defined(_WIN32) || defined(_WIN64) || defined(WIN_ENV)
    #define DllExport __declspec(dllexport)
  #else
    #define DllExport __attribute__((visibility("default")))
  #endif
#endif
#include "entry.h"
#include "AE_Effect.h"
#include "AE_EffectCB.h"
#include "AE_Macros.h"
#include "Param_Utils.h"
#include "String_Utils.h"
#endif // _STANDALONE_TEST_HARNESS_

#include "VHS6RIFT.h"
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <cstdio>
#include <cstring>

static PF_Err About(PF_OutData *out_data) {
  snprintf(out_data->return_msg, sizeof(out_data->return_msg), "%s 1.2 | Native VHS Master video processing for After Effects 23.x", VHS6RIFT_NAME);
  return PF_Err_NONE;
}

static PF_Err GlobalSetup(PF_OutData *out_data) {
  out_data->my_version = PF_VERSION(1, 2, 0, PF_Stage_RELEASE, 1);
  out_data->out_flags = PF_OutFlag_AUDIO_EFFECT_TOO | PF_OutFlag_AUDIO_FLOAT_ONLY;
  out_data->out_flags2 = 0;
  return PF_Err_NONE;
}

static PF_Err ParamsSetup(PF_InData *in_data, PF_OutData *out_data) {
  PF_Err err = PF_Err_NONE;
  PF_ParamDef def;

  AEFX_CLR_STRUCT(def);

  PF_ADD_FLOAT_SLIDER("Master VHS Strength", 0, 2000, 0, 2000, 350, 1, VHS_MASTER_STRENGTH);
  PF_ADD_FLOAT_SLIDER("Animation Speed", 0, 1000, 0, 1000, 100, 1, VHS_GLOBAL_ANIM_SPEED);
  PF_ADD_SLIDER("Random Seed", 0, 9999, 0, 9999, 17, VHS_GLOBAL_SEED);

  // 1. Camera Engines
  PF_ADD_TOPIC("1. Camera Engines", VHS_TOPIC_CAMERA_START);
  PF_ADD_POPUP("Camera Profile", 7, 1,
    "Fxc6 Reference|Gen 7 Reference|JVC GR-C7U (1986)|VHS-R20 Reference|Fall-78 Reference|ERC-V2S Reference|JVC GR-SXM740U", VHS_CAM_PROFILE);
  PF_ADD_FLOAT_SLIDER("Camera Strength", 0, 100, 0, 100, 100, 1, VHS_CAM_STRENGTH);
  PF_ADD_FLOAT_SLIDER("Exposure Shift", -50, 50, -50, 50, 0, 1, VHS_CAM_EXPOSURE);
  PF_ADD_FLOAT_SLIDER("Highlight Rolloff", 0, 100, 0, 100, 50, 1, VHS_CAM_HIGHLIGHT_ROLLOFF);
  PF_ADD_FLOAT_SLIDER("Shadow Lift", 0, 100, 0, 100, 10, 1, VHS_CAM_SHADOW_LIFT);
  PF_ADD_FLOAT_SLIDER("Color Saturation", 0, 200, 0, 200, 100, 1, VHS_CAM_COLOR_SAT);
  PF_ADD_FLOAT_SLIDER("WB Temp (Warm/Cool)", -50, 50, -50, 50, 0, 1, VHS_CAM_WB_TEMP);
  PF_ADD_FLOAT_SLIDER("WB Tint (Green/Magenta)", -50, 50, -50, 50, 0, 1, VHS_CAM_WB_TINT);
  PF_ADD_FLOAT_SLIDER("Optical Softness", 0, 100, 0, 100, 20, 1, VHS_CAM_OPTICAL_SOFTNESS);
  PF_ADD_FLOAT_SLIDER("Sharpness / Acutance", 0, 100, 0, 100, 30, 1, VHS_CAM_SHARPNESS);
  PF_ADD_FLOAT_SLIDER("Luma Bandwidth", 0, 100, 0, 100, 80, 1, VHS_CAM_LUMA_BANDWIDTH);
  PF_ADD_FLOAT_SLIDER("Chroma Bandwidth", 0, 100, 0, 100, 60, 1, VHS_CAM_CHROMA_BANDWIDTH);
  PF_ADD_FLOAT_SLIDER("Chroma Bleed Amount", 0, 100, 0, 100, 40, 1, VHS_CAM_CHROMA_BLEED);
  PF_ADD_FLOAT_SLIDER("Lowlight Noise Floor", 0, 100, 0, 100, 15, 1, VHS_CAM_LOWLIGHT_NOISE);
  PF_ADD_FLOAT_SLIDER("Format Response Drive", 0, 100, 0, 100, 50, 1, VHS_CAM_FORMAT_RESP);
  PF_END_TOPIC(VHS_TOPIC_CAMERA_END);

  // 2. VHS Degradation Engine
  PF_ADD_TOPIC("2. VHS Degradation Engine", VHS_TOPIC_TAPE_START);
  PF_ADD_POPUP("VHS Processing Mode", 4, 1, "SP Standard|LP Long Play|EP Extended Play|Hi-Fi Master", VHS_TAPE_MODE);
  PF_ADD_FLOAT_SLIDER("Tape Strength", 0, 100, 0, 100, 50, 1, VHS_TAPE_STRENGTH);
  PF_ADD_FLOAT_SLIDER("Tape Band Size", 1, 100, 1, 100, 20, 1, VHS_TAPE_BAND_SIZE);
  PF_ADD_FLOAT_SLIDER("Tape Transport Speed", 0, 300, 0, 300, 60, 1, VHS_TAPE_SPEED);
  PF_ADD_FLOAT_SLIDER("Phase Jitter", 0, 100, 0, 100, 25, 1, VHS_TAPE_JITTER);
  PF_ADD_FLOAT_SLIDER("Dropout Density", 0, 100, 0, 100, 15, 1, VHS_TAPE_DROPOUT_DENSITY);
  PF_ADD_FLOAT_SLIDER("Wrinkle Distortion", 0, 100, 0, 100, 10, 1, VHS_TAPE_WRINKLE_AMOUNT);
  PF_ADD_FLOAT_SLIDER("Chroma Deformation", 0, 100, 0, 100, 30, 1, VHS_TAPE_CHROMA_DEFORMATION);
  PF_END_TOPIC(VHS_TOPIC_TAPE_END);

  // 3. Bad Signal / Glitch Engine
  PF_ADD_TOPIC("3. Bad Signal / Glitch Engine", VHS_TOPIC_SUDDEN_GLITCH_START);
  PF_ADD_CHECKBOX("Enable Glitch Engine", "Enable", TRUE, 0, VHS_GLITCH_ENABLE);
  PF_ADD_FLOAT_SLIDER("Glitch Frequency", 0, 100, 0, 100, 30, 1, VHS_GLITCH_FREQ);
  PF_ADD_FLOAT_SLIDER("Glitch Duration (s)", 0.02, 2.0, 0.02, 2.0, 0.20, 2, VHS_GLITCH_DURATION);
  PF_ADD_FLOAT_SLIDER("Glitch Intensity", 0, 100, 0, 100, 60, 1, VHS_GLITCH_INTENSITY);
  PF_ADD_FLOAT_SLIDER("Pattern Variation", 0, 100, 0, 100, 40, 1, VHS_GLITCH_VARIATION);
  PF_ADD_FLOAT_SLIDER("Signal Recovery Speed", 0, 2.0, 0, 2.0, 0.15, 2, VHS_GLITCH_RECOVERY);
  PF_ADD_FLOAT_SLIDER("Horizontal Tearing", 0, 100, 0, 100, 50, 1, VHS_GLITCH_TEARING);
  PF_ADD_FLOAT_SLIDER("Band Disruption", 0, 100, 0, 100, 35, 1, VHS_GLITCH_BAND_DISRUPT);
  PF_ADD_FLOAT_SLIDER("Tracking Fail Rate", 0, 100, 0, 100, 25, 1, VHS_GLITCH_TRACKING_FAIL);
  PF_ADD_FLOAT_SLIDER("Chroma Instability", 0, 100, 0, 100, 45, 1, VHS_GLITCH_CHROMA_INSTAB);
  PF_ADD_FLOAT_SLIDER("Burst Flicker", 0, 100, 0, 100, 20, 1, VHS_GLITCH_BURST_FLICKER);
  PF_ADD_FLOAT_SLIDER("Glitch Speed", 0, 300, 0, 300, 100, 1, VHS_GLITCH_SPEED);
  PF_ADD_SLIDER("Glitch Seed", 0, 9999, 0, 9999, 1337, VHS_GLITCH_SEED);
  PF_END_TOPIC(VHS_TOPIC_SUDDEN_GLITCH_END);

  // 4. CRT TV Engine
  PF_ADD_TOPIC("4. CRT TV Engine", VHS_TOPIC_FILTERS_START);
  PF_ADD_POPUP("CRT Display Style", 4, 1, "Off|Trinitron CRT|Consumer TV|JVC Studio Monitor", VHS_FILTER_PRESET);
  PF_ADD_FLOAT_SLIDER("CRT Scanlines", 0, 100, 0, 100, 35, 1, VHS_FILTER_INTENSITY);
  PF_ADD_FLOAT_SLIDER("Phosphor Glow / Bloom", 0, 100, 0, 100, 25, 1, VHS_FILTER_WARMTH);
  PF_ADD_FLOAT_SLIDER("Screen Curvature", 0, 100, 0, 100, 20, 1, VHS_FILTER_GHOSTING);
  PF_ADD_FLOAT_SLIDER("RGB Convergence Shift", 0, 100, 0, 100, 18, 1, VHS_FILTER_RINGING);
  PF_ADD_FLOAT_SLIDER("CRT Flicker & Vignette", 0, 100, 0, 100, 15, 1, VHS_FILTER_BLACK_CRUSH);
  PF_END_TOPIC(VHS_TOPIC_FILTERS_END);

  // 5. Grain Engine
  PF_ADD_TOPIC("5. Grain Engine", VHS_TOPIC_GRAIN_START);
  PF_ADD_CHECKBOX("Enable Grain", "Enable", TRUE, 0, VHS_GRAIN_ENABLE);
  PF_ADD_POPUP("Grain Mode", 3, 1, "Monochrome|Color Organic|RF Hiss Noise", VHS_GRAIN_MODE);
  PF_ADD_FLOAT_SLIDER("Grain Amount", 0, 100, 0, 100, 25, 1, VHS_GRAIN_AMOUNT);
  PF_ADD_FLOAT_SLIDER("Luma Grain Strength", 0, 100, 0, 100, 20, 1, VHS_GRAIN_LUMA_INTENSITY);
  PF_ADD_FLOAT_SLIDER("Chroma Grain Strength", 0, 100, 0, 100, 25, 1, VHS_GRAIN_CHROMA_INTENSITY);
  PF_ADD_FLOAT_SLIDER("Grain Size / Texture", 1, 100, 1, 100, 25, 1, VHS_GRAIN_SIZE);
  PF_ADD_FLOAT_SLIDER("Grain Softness", 0, 100, 0, 100, 30, 1, VHS_GRAIN_SOFTNESS);
  PF_ADD_FLOAT_SLIDER("Shadow Noise Response", 0, 100, 0, 100, 40, 1, VHS_GRAIN_SHADOW_RESP);
  PF_ADD_FLOAT_SLIDER("Midtone Noise Response", 0, 100, 0, 100, 30, 1, VHS_GRAIN_MIDTONE_RESP);
  PF_ADD_FLOAT_SLIDER("Highlight Noise Response", 0, 100, 0, 100, 15, 1, VHS_GRAIN_HIGHLIGHT_RESP);
  PF_ADD_FLOAT_SLIDER("Grain Speed", 0, 400, 0, 400, 100, 1, VHS_GRAIN_MOTION_SPEED);
  PF_ADD_SLIDER("Grain Seed", 0, 9999, 0, 9999, 42, VHS_GRAIN_SEED);
  PF_END_TOPIC(VHS_TOPIC_GRAIN_END);

  // 6. Dates & Text Overlay Engine
  PF_ADD_TOPIC("6. Dates & Text OSD", VHS_TOPIC_TEXT_START);
  PF_ADD_CHECKBOX("Enable Date/Text OSD", "Enable", TRUE, 0, VHS_TEXT_ENABLE);
  PF_ADD_POPUP("OSD Style", 5, 1, "Date & Time|PLAY + Date|REC + Timestamp|AUTO SP|Custom Text Preset", VHS_TEXT_STYLE);
  PF_ADD_SLIDER("Year", 1970, 2030, 1970, 2030, 1994, VHS_TEXT_YEAR);
  PF_ADD_SLIDER("Month", 1, 12, 1, 12, 5, VHS_TEXT_MONTH);
  PF_ADD_SLIDER("Day", 1, 31, 1, 31, 18, VHS_TEXT_DAY);
  PF_ADD_SLIDER("Hour", 0, 23, 0, 23, 12, VHS_TEXT_HOUR);
  PF_ADD_SLIDER("Minute", 0, 59, 0, 59, 34, VHS_TEXT_MINUTE);
  PF_ADD_POPUP("Preset String", 6, 1, "PLAY|REC ●|PAUSE|AUTO SP|STOP|VHS MASTER", VHS_TEXT_PRESET_STRING);
  PF_ADD_FLOAT_SLIDER("Position X (%)", 0, 100, 0, 100, 8, 1, VHS_TEXT_POS_X);
  PF_ADD_FLOAT_SLIDER("Position Y (%)", 0, 100, 0, 100, 88, 1, VHS_TEXT_POS_Y);
  PF_ADD_FLOAT_SLIDER("Text Scale", 50, 300, 50, 300, 100, 1, VHS_TEXT_SCALE);
  PF_ADD_FLOAT_SLIDER("Text Opacity", 0, 100, 0, 100, 90, 1, VHS_TEXT_OPACITY);
  PF_END_TOPIC(VHS_TOPIC_TEXT_END);

  // 7. Advanced Overlays & Audio
  PF_ADD_TOPIC("7. Advanced Overlays & Audio", VHS_TOPIC_DAMAGE_START);
  PF_ADD_CHECKBOX("Enable Layer Overlay", "Enable", FALSE, 0, VHS_DAMAGE_ENABLE);
  PF_ADD_POPUP("Damage Preset", 4, 1, "Transparent Tape|Moving Sweep|Oxide Dust|Glitch Bars", VHS_DAMAGE_TYPE);
  PF_ADD_FLOAT_SLIDER("Damage Opacity", 0, 100, 0, 100, 50, 1, VHS_DAMAGE_OPACITY);
  PF_ADD_FLOAT_SLIDER("Damage Scale", 10, 200, 10, 200, 100, 1, VHS_DAMAGE_SCALE);
  PF_ADD_FLOAT_SLIDER("Damage Speed", 0, 300, 0, 300, 100, 1, VHS_DAMAGE_SPEED);
  PF_ADD_POPUP("Blend Mode", 6, 1, "Normal|Blend|Add|Screen|Multiply|Luma Key", VHS_DAMAGE_BLEND_MODE);
  PF_ADD_LAYER("Overlay Layer", PF_LayerDefault_NONE, VHS_DAMAGE_LAYER);
  PF_ADD_FLOAT_SLIDER("Loop Period (s)", 0.1, 60, 0.1, 60, 5, 1, VHS_DAMAGE_LOOP_PERIOD);
  PF_END_TOPIC(VHS_TOPIC_DAMAGE_END);

  PF_ADD_TOPIC("Other Signal Engines", VHS_TOPIC_OTHER_START);
  PF_ADD_POPUP("Secondary VHS Engine", 5, 1, "Off|VHS Craft|VHS Legend|NTSC-RS Model|Signal Model", VHS_OTHER_ENGINE);
  PF_ADD_FLOAT_SLIDER("Secondary Strength", 0, 100, 0, 100, 50, 1, VHS_OTHER_STRENGTH);
  PF_ADD_FLOAT_SLIDER("Phase Drift", 0, 100, 0, 100, 20, 1, VHS_OTHER_PHASE_DRIFT);
  PF_ADD_FLOAT_SLIDER("RF Carrier Noise", 0, 100, 0, 100, 15, 1, VHS_OTHER_RF_NOISE);
  PF_ADD_FLOAT_SLIDER("Edge Ringing", 0, 100, 0, 100, 25, 1, VHS_OTHER_RINGING);
  PF_ADD_FLOAT_SLIDER("Chroma Crosstalk", 0, 100, 0, 100, 30, 1, VHS_OTHER_CROSSTALK);
  PF_END_TOPIC(VHS_TOPIC_OTHER_END);

  PF_ADD_TOPIC("Audio Processing", VHS_TOPIC_AUDIO_START);
  PF_ADD_CHECKBOX("Enable Audio Processing", "Enable", FALSE, 0, VHS_AUDIO_ENABLE);
  PF_ADD_CHECKBOX("Audio Glitch Events", "Enable", FALSE, 0, VHS_AUDIO_EVENT_ENABLE);
  PF_ADD_FLOAT_SLIDER("Audio Mix (Wet/Dry)", 0, 100, 0, 100, 50, 1, VHS_AUDIO_WET_DRY);
  PF_ADD_FLOAT_SLIDER("Output Gain", -20, 20, -20, 20, 0, 1, VHS_AUDIO_OUTPUT_GAIN);
  PF_ADD_FLOAT_SLIDER("Tape Saturation", 0, 100, 0, 100, 20, 1, VHS_AUDIO_DISTORTION);
  PF_ADD_FLOAT_SLIDER("Lo-Fi Quantization", 0, 100, 0, 100, 10, 1, VHS_AUDIO_BIT_DEPTH);
  PF_ADD_FLOAT_SLIDER("Sample Rate Reduction", 0, 100, 0, 100, 10, 1, VHS_AUDIO_SAMPLE_RATE);
  PF_ADD_FLOAT_SLIDER("Crackle / Static", 0, 100, 0, 100, 15, 1, VHS_AUDIO_CRACKLE);
  PF_ADD_FLOAT_SLIDER("Audio Dropouts", 0, 100, 0, 100, 10, 1, VHS_AUDIO_DROPOUTS);
  PF_ADD_FLOAT_SLIDER("Flutter / Wow", 0, 100, 0, 100, 12, 1, VHS_AUDIO_FLUTTER);
  PF_ADD_SLIDER("Audio Seed", 0, 9999, 0, 9999, 88, VHS_AUDIO_SEED);
  PF_END_TOPIC(VHS_TOPIC_AUDIO_END);

  out_data->num_params = VHS_NUM_PARAMS;
  return err;
}

static inline float clamp01(float v) { return std::max(0.0f, std::min(1.0f, v)); }
static inline uint32_t hash32(uint32_t x) {
  x ^= x >> 16; x *= 0x7feb352dU; x ^= x >> 15; x *= 0x846ca68bU; x ^= x >> 16; return x;
}
static inline float rand01(uint32_t x) { return (hash32(x) & 0x00ffffffU) / 16777215.0f; }
static inline float smooth(float a, float b, float x) { float t = clamp01((x-a)/(b-a)); return t*t*(3.0f-2.0f*t); }
static inline int clampi(int v, int a, int b) { return std::max(a, std::min(b, v)); }

// 5x7 ASCII bitmap font definitions for camcorder OSD rendering
static const uint8_t font5x7[][5] = {
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // ' ' (32)
  { 0x00, 0x00, 0x5F, 0x00, 0x00 }, // '!'
  { 0x00, 0x07, 0x00, 0x07, 0x00 }, // '"'
  { 0x14, 0x7F, 0x14, 0x7F, 0x14 }, // '#'
  { 0x24, 0x2A, 0x7F, 0x2A, 0x12 }, // '$'
  { 0x23, 0x13, 0x08, 0x64, 0x62 }, // '%'
  { 0x36, 0x49, 0x55, 0x22, 0x50 }, // '&'
  { 0x00, 0x05, 0x03, 0x00, 0x00 }, // '\''
  { 0x00, 0x1C, 0x22, 0x41, 0x00 }, // '('
  { 0x00, 0x41, 0x22, 0x1C, 0x00 }, // ')'
  { 0x14, 0x08, 0x3E, 0x08, 0x14 }, // '*'
  { 0x08, 0x08, 0x3E, 0x08, 0x08 }, // '+'
  { 0x00, 0x50, 0x30, 0x00, 0x00 }, // ','
  { 0x08, 0x08, 0x08, 0x08, 0x08 }, // '-'
  { 0x00, 0x60, 0x60, 0x00, 0x00 }, // '.'
  { 0x20, 0x10, 0x08, 0x04, 0x02 }, // '/'
  { 0x3E, 0x51, 0x49, 0x45, 0x3E }, // '0' (48)
  { 0x00, 0x42, 0x7F, 0x40, 0x00 }, // '1'
  { 0x42, 0x61, 0x51, 0x49, 0x46 }, // '2'
  { 0x21, 0x41, 0x45, 0x4B, 0x31 }, // '3'
  { 0x18, 0x14, 0x12, 0x7F, 0x10 }, // '4'
  { 0x27, 0x45, 0x45, 0x45, 0x39 }, // '5'
  { 0x3C, 0x4A, 0x49, 0x49, 0x30 }, // '6'
  { 0x01, 0x71, 0x09, 0x05, 0x03 }, // '7'
  { 0x36, 0x49, 0x49, 0x49, 0x36 }, // '8'
  { 0x06, 0x49, 0x49, 0x29, 0x1E }, // '9'
  { 0x00, 0x36, 0x36, 0x00, 0x00 }, // ':'
  { 0x00, 0x56, 0x36, 0x00, 0x00 }, // ';'
  { 0x08, 0x14, 0x22, 0x41, 0x00 }, // '<'
  { 0x14, 0x14, 0x14, 0x14, 0x14 }, // '='
  { 0x00, 0x41, 0x22, 0x14, 0x08 }, // '>'
  { 0x02, 0x01, 0x51, 0x09, 0x06 }, // '?'
  { 0x32, 0x49, 0x79, 0x41, 0x3E }, // '@'
  { 0x7E, 0x11, 0x11, 0x11, 0x7E }, // 'A' (65)
  { 0x7F, 0x49, 0x49, 0x49, 0x36 }, // 'B'
  { 0x3E, 0x41, 0x41, 0x41, 0x22 }, // 'C'
  { 0x7F, 0x41, 0x41, 0x22, 0x1C }, // 'D'
  { 0x7F, 0x49, 0x49, 0x49, 0x41 }, // 'E'
  { 0x7F, 0x09, 0x09, 0x09, 0x01 }, // 'F'
  { 0x3E, 0x41, 0x49, 0x49, 0x7A }, // 'G'
  { 0x7F, 0x08, 0x08, 0x08, 0x7F }, // 'H'
  { 0x00, 0x41, 0x7F, 0x41, 0x00 }, // 'I'
  { 0x20, 0x40, 0x41, 0x3F, 0x01 }, // 'J'
  { 0x7F, 0x08, 0x14, 0x22, 0x41 }, // 'K'
  { 0x7F, 0x40, 0x40, 0x40, 0x40 }, // 'L'
  { 0x7F, 0x02, 0x0C, 0x02, 0x7F }, // 'M'
  { 0x7F, 0x04, 0x08, 0x10, 0x7F }, // 'N'
  { 0x3E, 0x41, 0x41, 0x41, 0x3E }, // 'O'
  { 0x7F, 0x09, 0x09, 0x09, 0x06 }, // 'P'
  { 0x3E, 0x41, 0x51, 0x21, 0x5E }, // 'Q'
  { 0x7F, 0x09, 0x19, 0x29, 0x46 }, // 'R'
  { 0x46, 0x49, 0x49, 0x49, 0x31 }, // 'S'
  { 0x01, 0x01, 0x7F, 0x01, 0x01 }, // 'T'
  { 0x3F, 0x40, 0x40, 0x40, 0x3F }, // 'U'
  { 0x1F, 0x20, 0x40, 0x20, 0x1F }, // 'V'
  { 0x3F, 0x40, 0x38, 0x40, 0x3F }, // 'W'
  { 0x63, 0x14, 0x08, 0x14, 0x63 }, // 'X'
  { 0x07, 0x08, 0x70, 0x08, 0x07 }, // 'Y'
  { 0x61, 0x51, 0x49, 0x45, 0x43 }  // 'Z'
};

static void DrawOSDText(PF_Pixel *pixels, int w, int h, int stride, const char *str, int startX, int startY, float scale, float opacity) {
  if (!str || opacity <= 0.001f) return;
  int fontScale = clampi((int)std::round(scale), 1, 6);
  int charW = 6 * fontScale;
  int charH = 8 * fontScale;
  int xCursor = startX;
  int yCursor = startY;

  for (size_t i = 0; i < strlen(str); ++i) {
    char c = str[i];
    if (c == '\n') {
      yCursor += charH + 2;
      xCursor = startX;
      continue;
    }
    int fontIndex = 0;
    if (c >= ' ' && c <= 'Z') fontIndex = c - ' ';
    else if (c >= 'a' && c <= 'z') fontIndex = (c - 'a') + ('A' - ' ');

    for (int col = 0; col < 5; ++col) {
      uint8_t bits = font5x7[fontIndex][col];
      for (int row = 0; row < 7; ++row) {
        if ((bits >> row) & 1) {
          for (int sy = 0; sy < fontScale; ++sy) {
            for (int sx = 0; sx < fontScale; ++sx) {
              int px = xCursor + col * fontScale + sx;
              int py = yCursor + row * fontScale + sy;
              if (px >= 0 && px < w && py >= 0 && py < h) {
                PF_Pixel *p = pixels + py * stride + px;
                float op = opacity;
                p->red = (uint8_t)clampi((int)(p->red * (1.0f - op) + 255.0f * op), 0, 255);
                p->green = (uint8_t)clampi((int)(p->green * (1.0f - op) + 255.0f * op), 0, 255);
                p->blue = (uint8_t)clampi((int)(p->blue * (1.0f - op) + 255.0f * op), 0, 255);
              }
            }
          }
        }
      }
    }
    xCursor += charW;
  }
}

static float getSlider(PF_ParamDef *params[], int id, float scale) { return (float)params[id]->u.fs_d.value / scale; }
static int getInt(PF_ParamDef *params[], int id) { return params[id]->u.sd.value; }
static bool getCheck(PF_ParamDef *params[], int id) { return params[id]->u.bd.value != 0; }

static PF_Err Render(PF_InData *in_data, PF_ParamDef *params[], PF_LayerDef *output) {
  PF_Err err = PF_Err_NONE;
  PF_EffectWorld *src = &params[VHS_INPUT]->u.ld;
  if (!src || !src->data || !output->data) return PF_Err_NONE;

  const int w = output->width, h = output->height;
  const int inW = src->width, inH = src->height;
  const PF_Pixel *inBase = (const PF_Pixel*)src->data;
  PF_Pixel *outBase = (PF_Pixel*)output->data;
  const int inStride = src->rowbytes / (int)sizeof(PF_Pixel);
  const int outStride = output->rowbytes / (int)sizeof(PF_Pixel);

  const float animSpeed = getSlider(params, VHS_GLOBAL_ANIM_SPEED, 100.0f);
  const int globalSeed = getInt(params, VHS_GLOBAL_SEED);
  const float timeSec = (in_data->time_scale ? (float)in_data->current_time / (float)in_data->time_scale : 0.0f) * animSpeed;

  const float master = getSlider(params, VHS_MASTER_STRENGTH, 100.0f);
  const float strength = std::min(20.0f, std::max(0.0f, master));

  // 1. Camera Engine Profile coefficients
  const int cameraProfile = clampi(params[VHS_CAM_PROFILE]->u.pd.value - 1, 0, 6);
  const float camStrength = getSlider(params, VHS_CAM_STRENGTH, 100.0f);
  const float camExposure = getSlider(params, VHS_CAM_EXPOSURE, 50.0f); // -1.0 to +1.0
  const float camRolloff = getSlider(params, VHS_CAM_HIGHLIGHT_ROLLOFF, 100.0f);
  const float camShadowLift = getSlider(params, VHS_CAM_SHADOW_LIFT, 100.0f) * 0.15f;
  const float camSat = getSlider(params, VHS_CAM_COLOR_SAT, 100.0f);
  const float camWBTemp = getSlider(params, VHS_CAM_WB_TEMP, 50.0f) * 0.10f;
  const float camWBTint = getSlider(params, VHS_CAM_WB_TINT, 50.0f) * 0.10f;
  const float camSoftness = getSlider(params, VHS_CAM_OPTICAL_SOFTNESS, 100.0f);
  const float camSharpness = getSlider(params, VHS_CAM_SHARPNESS, 100.0f);
  const float camLumaBW = getSlider(params, VHS_CAM_LUMA_BANDWIDTH, 100.0f);
  const float camChromaBW = getSlider(params, VHS_CAM_CHROMA_BANDWIDTH, 100.0f);
  const float camBleed = getSlider(params, VHS_CAM_CHROMA_BLEED, 100.0f);
  const float camNoiseFloor = getSlider(params, VHS_CAM_LOWLIGHT_NOISE, 100.0f);
  const float camFormatResp = getSlider(params, VHS_CAM_FORMAT_RESP, 100.0f);

  // Profile-specific color gains & warmth
  static const float pWarm[7] = { 0.03f, -0.02f, 0.06f, 0.01f, 0.08f, 0.00f, 0.02f };
  static const float pGreen[7] = { 0.01f, 0.03f, 0.01f, 0.00f, -0.02f, 0.01f, 0.00f };
  static const float pGainR[7] = { 1.04f, 0.98f, 1.12f, 1.01f, 1.15f, 1.00f, 1.02f };
  static const float pGainG[7] = { 0.99f, 1.02f, 1.03f, 1.00f, 0.96f, 1.00f, 0.99f };
  static const float pGainB[7] = { 0.96f, 1.03f, 0.92f, 0.98f, 0.90f, 1.01f, 1.00f };

  // 2. VHS Engine Parameters
  const int tapeMode = clampi(params[VHS_TAPE_MODE]->u.pd.value - 1, 0, 3);
  const float tapeStrength = getSlider(params, VHS_TAPE_STRENGTH, 100.0f);
  const float tapeBandSize = getSlider(params, VHS_TAPE_BAND_SIZE, 100.0f) * 0.15f;
  const float tapeSpeed = getSlider(params, VHS_TAPE_SPEED, 100.0f);
  const float tapeJitter = getSlider(params, VHS_TAPE_JITTER, 100.0f);
  const float tapeDropoutDensity = getSlider(params, VHS_TAPE_DROPOUT_DENSITY, 100.0f);
  const float tapeWrinkle = getSlider(params, VHS_TAPE_WRINKLE_AMOUNT, 100.0f);
  const float tapeChromaDeform = getSlider(params, VHS_TAPE_CHROMA_DEFORMATION, 100.0f);

  // Mode multipliers (LP/EP add more jitter & degradation)
  float modeJitterMult = 1.0f;
  if (tapeMode == 1) modeJitterMult = 1.8f;      // LP
  else if (tapeMode == 2) modeJitterMult = 2.8f; // EP
  else if (tapeMode == 3) modeJitterMult = 0.4f; // Hi-Fi Master

  // 3. Glitch Engine Parameters
  const bool glitchEnabled = getCheck(params, VHS_GLITCH_ENABLE);
  const float glitchFreq = getSlider(params, VHS_GLITCH_FREQ, 100.0f);
  const float glitchDuration = getSlider(params, VHS_GLITCH_DURATION, 1.0f);
  const float glitchIntensity = glitchEnabled ? getSlider(params, VHS_GLITCH_INTENSITY, 100.0f) : 0.0f;
  const float glitchVar = getSlider(params, VHS_GLITCH_VARIATION, 100.0f);
  const float glitchRecovery = getSlider(params, VHS_GLITCH_RECOVERY, 1.0f);
  const float glitchTearing = getSlider(params, VHS_GLITCH_TEARING, 100.0f);
  const float glitchBandDisrupt = getSlider(params, VHS_GLITCH_BAND_DISRUPT, 100.0f);
  const float glitchTrackFail = getSlider(params, VHS_GLITCH_TRACKING_FAIL, 100.0f);
  const float glitchChromaInstab = getSlider(params, VHS_GLITCH_CHROMA_INSTAB, 100.0f);
  const float glitchFlicker = getSlider(params, VHS_GLITCH_BURST_FLICKER, 100.0f);
  const float glitchSpeed = getSlider(params, VHS_GLITCH_SPEED, 100.0f);
  const int glitchSeed = getInt(params, VHS_GLITCH_SEED);

  // Time-based glitch event window computation
  float glitchEnv = 0.0f;
  if (glitchEnabled && glitchIntensity > 0.001f && glitchFreq > 0.001f) {
    float eventPeriod = std::max(0.1f, 10.0f / glitchFreq);
    int slot = (int)std::floor((timeSec * glitchSpeed * 0.01f) / eventPeriod);
    float slotTime = (timeSec * glitchSpeed * 0.01f) - slot * eventPeriod;
    uint32_t slotHash = hash32((uint32_t)slot ^ (uint32_t)glitchSeed ^ (uint32_t)(glitchVar * 10.0f));
    bool trigger = (rand01(slotHash) < (glitchFreq * 0.008f));
    if (trigger && slotTime < glitchDuration) {
      glitchEnv = glitchIntensity * (1.0f - (slotTime / (glitchDuration * (1.0f + glitchRecovery))));
    }
  }

  // 4. CRT TV Engine
  const int crtPreset = params[VHS_FILTER_PRESET]->u.pd.value - 1;
  const bool crtEnabled = (crtPreset > 0);
  const float crtScanlines = getSlider(params, VHS_FILTER_INTENSITY, 100.0f);
  const float crtBloom = getSlider(params, VHS_FILTER_WARMTH, 100.0f);
  const float crtCurve = getSlider(params, VHS_FILTER_GHOSTING, 100.0f);
  const float crtRgb = getSlider(params, VHS_FILTER_RINGING, 100.0f);
  const float crtFlicker = getSlider(params, VHS_FILTER_BLACK_CRUSH, 100.0f);

  // 5. Grain Engine
  const bool grainEnabled = getCheck(params, VHS_GRAIN_ENABLE);
  const int grainMode = params[VHS_GRAIN_MODE]->u.pd.value - 1;
  const float grainAmount = grainEnabled ? getSlider(params, VHS_GRAIN_AMOUNT, 100.0f) : 0.0f;
  const float grainLuma = getSlider(params, VHS_GRAIN_LUMA_INTENSITY, 100.0f);
  const float grainChroma = getSlider(params, VHS_GRAIN_CHROMA_INTENSITY, 100.0f);
  const float grainSize = std::max(1.0f, getSlider(params, VHS_GRAIN_SIZE, 100.0f) * 4.0f);
  const float grainSoftness = getSlider(params, VHS_GRAIN_SOFTNESS, 100.0f);
  const float grainShadow = getSlider(params, VHS_GRAIN_SHADOW_RESP, 100.0f);
  const float grainMid = getSlider(params, VHS_GRAIN_MIDTONE_RESP, 100.0f);
  const float grainHighlight = getSlider(params, VHS_GRAIN_HIGHLIGHT_RESP, 100.0f);
  const float grainSpeed = getSlider(params, VHS_GRAIN_MOTION_SPEED, 100.0f);
  const int grainSeed = getInt(params, VHS_GRAIN_SEED);

  // 7. Advanced Overlays & Secondary
  const bool damageEnabled = getCheck(params, VHS_DAMAGE_ENABLE);
  const int damagePreset = params[VHS_DAMAGE_TYPE]->u.pd.value - 1;
  const float damageOpacity = getSlider(params, VHS_DAMAGE_OPACITY, 100.0f);
  const float damageScale = getSlider(params, VHS_DAMAGE_SCALE, 100.0f);
  const float damageSpeed = getSlider(params, VHS_DAMAGE_SPEED, 100.0f);
  const int damageBlend = params[VHS_DAMAGE_BLEND_MODE]->u.pd.value - 1;
  const float damageLoop = getSlider(params, VHS_DAMAGE_LOOP_PERIOD, 1.0f);

  PF_ParamDef overlayCheckout;
  AEFX_CLR_STRUCT(overlayCheckout);
  PF_EffectWorld *overlayWorld = nullptr;
  if (damageEnabled) {
    PF_Err checkoutErr = PF_CHECKOUT_PARAM(in_data, VHS_DAMAGE_LAYER, in_data->current_time, in_data->time_step, in_data->time_scale, &overlayCheckout);
    if (checkoutErr == PF_Err_NONE && overlayCheckout.u.ld.data) {
      overlayWorld = &overlayCheckout.u.ld;
    }
  }

  const int otherEngine = params[VHS_OTHER_ENGINE]->u.pd.value - 1;
  const float otherStrength = getSlider(params, VHS_OTHER_STRENGTH, 100.0f);
  const float otherDrift = getSlider(params, VHS_OTHER_PHASE_DRIFT, 100.0f);
  const float otherRF = getSlider(params, VHS_OTHER_RF_NOISE, 100.0f);
  const float otherRinging = getSlider(params, VHS_OTHER_RINGING, 100.0f);
  const float otherCrosstalk = getSlider(params, VHS_OTHER_CROSSTALK, 100.0f);

  // Compute moving tape band center position driven by tapeSpeed
  float bandCenterY = std::fmod(timeSec * (0.05f + tapeSpeed * 0.005f), 1.0f);

  for (int y = 0; y < h; ++y) {
    PF_Pixel *dst = outBase + y * outStride;
    float yn = (float)y / std::max(1, h);

    // Tape Band Displacement
    float bandDist = std::fabs(yn - bandCenterY);
    float inTapeBand = (bandDist < tapeBandSize) ? (1.0f - bandDist / tapeBandSize) : 0.0f;

    for (int x = 0; x < w; ++x) {
      int sx = clampi((int)((x + 0.5f) * inW / std::max(1, w)), 0, inW - 1);
      int sy = clampi((int)((y + 0.5f) * inH / std::max(1, h)), 0, inH - 1);

      // CRT Barrel Curvature Geometry Distortion
      if (crtEnabled && crtCurve > 0.001f) {
        float nx = ((float)x / std::max(1, w - 1) - 0.5f) * 2.0f;
        float ny = ((float)y / std::max(1, h - 1) - 0.5f) * 2.0f;
        float rr = nx * nx + ny * ny;
        float k = 1.0f + crtCurve * 0.12f * rr;
        sx = clampi((int)std::round(((nx * k * 0.5f + 0.5f) * inW)), 0, inW - 1);
        sy = clampi((int)std::round(((ny * k * 0.5f + 0.5f) * inH)), 0, inH - 1);
      }

      // Glitch Horizontal Line Tearing & Band Displacement
      if (glitchEnv > 0.001f) {
        uint32_t lineHash = hash32((uint32_t)sy ^ (uint32_t)(timeSec * 30.0f) ^ (uint32_t)glitchSeed);
        if (rand01(lineHash) < (glitchTearing * 0.01f * glitchEnv)) {
          int offset = (int)std::round((rand01(lineHash + 1) - 0.5f) * glitchBandDisrupt * 0.5f * inW);
          sx = clampi(sx + offset, 0, inW - 1);
        }
      }

      // Tape Phase Jitter Row Displacement & Wrinkle Distortion
      if (tapeStrength > 0.001f && (tapeJitter > 0.001f || tapeWrinkle > 0.001f || inTapeBand > 0.001f)) {
        uint32_t jHash = hash32((uint32_t)sy ^ (uint32_t)(timeSec * 24.0f) ^ (uint32_t)globalSeed);
        int jShift = (int)std::round((rand01(jHash) - 0.5f) * tapeJitter * 0.12f * modeJitterMult * tapeStrength);

        // Wrinkle sinusoidal displacement
        if (tapeWrinkle > 0.001f) {
          jShift += (int)std::round(std::sin(sy * 0.05f + timeSec * 10.0f) * tapeWrinkle * 0.15f * tapeStrength);
        }

        // Tape band displacement
        if (inTapeBand > 0.001f) {
          jShift += (int)std::round((rand01(jHash + 10) - 0.5f) * inTapeBand * 20.0f * tapeStrength);
        }

        sx = clampi(sx + jShift, 0, inW - 1);
      }

      PF_Pixel p = inBase[sy * inStride + sx];
      float r = p.red / 255.0f, g = p.green / 255.0f, b = p.blue / 255.0f;

      // Optical Softness & Sharpness Spatial Filter
      if (camSoftness > 0.001f || camSharpness > 0.001f) {
        int leftX = clampi(sx - 1, 0, inW - 1);
        int rightX = clampi(sx + 1, 0, inW - 1);
        PF_Pixel pL = inBase[sy * inStride + leftX];
        PF_Pixel pR = inBase[sy * inStride + rightX];
        float avgR = (pL.red + pR.red + p.red) / (3.0f * 255.0f);
        float avgG = (pL.green + pR.green + p.green) / (3.0f * 255.0f);
        float avgB = (pL.blue + pR.blue + p.blue) / (3.0f * 255.0f);

        if (camSoftness > 0.001f) {
          float sFactor = camSoftness * 0.005f;
          r = r * (1.0f - sFactor) + avgR * sFactor;
          g = g * (1.0f - sFactor) + avgG * sFactor;
          b = b * (1.0f - sFactor) + avgB * sFactor;
        }
        if (camSharpness > 0.001f) {
          float shFactor = camSharpness * 0.005f;
          r = clamp01(r + (r - avgR) * shFactor);
          g = clamp01(g + (g - avgG) * shFactor);
          b = clamp01(b + (b - avgB) * shFactor);
        }
      }

      // Magnetic Oxide Dropouts (Pixel Streaks)
      if (tapeStrength > 0.001f && tapeDropoutDensity > 0.001f) {
        uint32_t dropHash = hash32((uint32_t)sx ^ (uint32_t)(sy * 73) ^ (uint32_t)(timeSec * 15.0f));
        if (rand01(dropHash) < (tapeDropoutDensity * 0.0005f * tapeStrength)) {
          r = 0.85f; g = 0.85f; b = 0.85f; // Oxide dropout white streak
        }
      }

      // Lowlight Noise Floor
      if (camNoiseFloor > 0.001f) {
        float lum = 0.2126f * r + 0.7152f * g + 0.0722f * b;
        if (lum < 0.35f) {
          uint32_t nHash = hash32((uint32_t)x ^ (uint32_t)(y * 131) ^ (uint32_t)(timeSec * 24.0f));
          float nVal = (rand01(nHash) - 0.5f) * camNoiseFloor * 0.002f * (0.35f - lum);
          r = clamp01(r + nVal); g = clamp01(g + nVal); b = clamp01(b + nVal);
        }
      }

      // 1. Camera Engine Processing
      if (camStrength > 0.001f) {
        // Exposure Shift & White Balance
        r = std::max(0.0f, r + camExposure + camWBTemp);
        g = std::max(0.0f, g + camExposure - camWBTint);
        b = std::max(0.0f, b + camExposure - camWBTemp);

        // Profile Gain & Warmth Adjustment
        float pMix = camStrength * 0.01f;
        r *= (1.0f - pMix) + pGainR[cameraProfile] * pMix + pWarm[cameraProfile] * pMix;
        g *= (1.0f - pMix) + pGainG[cameraProfile] * pMix + pGreen[cameraProfile] * pMix;
        b *= (1.0f - pMix) + pGainB[cameraProfile] * pMix - pWarm[cameraProfile] * pMix;

        // Format Response Drive Gamma Curve
        if (camFormatResp != 50.0f) {
          float gamma = std::pow(2.0f, (camFormatResp - 50.0f) * 0.01f);
          r = std::pow(clamp01(r), gamma);
          g = std::pow(clamp01(g), gamma);
          b = std::pow(clamp01(b), gamma);
        }

        // Shadow Lift & Highlight Rolloff
        r = r + camShadowLift;
        g = g + camShadowLift;
        b = b + camShadowLift;

        if (camRolloff > 0.001f) {
          float shoulder = camRolloff * 0.002f;
          r -= shoulder * std::max(0.0f, r - 0.70f) * std::max(0.0f, r - 0.70f);
          g -= shoulder * std::max(0.0f, g - 0.70f) * std::max(0.0f, g - 0.70f);
          b -= shoulder * std::max(0.0f, b - 0.70f) * std::max(0.0f, b - 0.70f);
        }

        // Chroma Bleed & Chroma Deformation
        int cx = clampi(sx + (int)std::round((camBleed + tapeChromaDeform * 0.5f) * 0.08f * strength * pMix), 0, inW - 1);
        PF_Pixel cp = inBase[sy * inStride + cx];
        float cbR = cp.red / 255.0f, cbB = cp.blue / 255.0f;
        r = r * (1.0f - camBleed * 0.003f) + cbR * (camBleed * 0.003f);
        b = b * (1.0f - camBleed * 0.003f) + cbB * (camBleed * 0.003f);

        float lum = 0.2126f * r + 0.7152f * g + 0.0722f * b;
        float sat = camSat * (0.8f + strength * 0.05f);
        r = lum + (r - lum) * sat;
        g = lum + (g - lum) * sat;
        b = lum + (b - lum) * sat;
      }

      // Secondary VHS Engines (Craft, Legend, NTSC-RS, Signal Models)
      if (otherEngine > 0 && otherStrength > 0.001f) {
        float oMix = otherStrength * 0.01f;
        float drift = std::sin(sy * 0.1f + timeSec * (5.0f + otherDrift * 0.1f)) * otherStrength * 0.02f;
        uint32_t rfHash = hash32((uint32_t)sx ^ (uint32_t)(sy * 37) ^ (uint32_t)(timeSec * 29.0f));
        float rfNoise = (rand01(rfHash) - 0.5f) * otherRF * 0.003f;
        r = clamp01(r + drift + rfNoise + otherRinging * 0.001f);
        g = clamp01(g - drift * 0.5f + rfNoise);
        b = clamp01(b + drift + rfNoise + otherCrosstalk * 0.002f);
      }

      // 3. Glitch Burst Color Instability & Flicker
      if (glitchEnv > 0.001f) {
        r = clamp01(r + glitchChromaInstab * 0.005f * glitchEnv);
        b = clamp01(b + glitchFlicker * 0.004f * glitchEnv);
        if (glitchTrackFail > 0.001f && (sy % 16 < 4)) {
          r *= (1.0f - glitchTrackFail * 0.005f * glitchEnv);
          g *= (1.0f - glitchTrackFail * 0.005f * glitchEnv);
          b *= (1.0f - glitchTrackFail * 0.005f * glitchEnv);
        }
      }

      // 5. Procedural Grain Generation with Tonal Weighting & Grain Softness
      if (grainEnabled && grainAmount > 0.001f) {
        uint32_t motionFrame = (uint32_t)std::floor(timeSec * grainSpeed * 24.0f);
        int gx = (int)(x / grainSize);
        int gy = (int)(y / grainSize);
        uint32_t nseed = (uint32_t)grainSeed ^ (uint32_t)globalSeed ^ (motionFrame * 2246822519u) ^ (uint32_t)(gx * 73856093u) ^ (uint32_t)(gy * 19349663u);

        float lum = 0.2126f * r + 0.7152f * g + 0.0722f * b;
        float tonalWeight = (1.0f - lum) * (grainShadow * 0.02f) + (1.0f - std::fabs(lum - 0.5f) * 2.0f) * (grainMid * 0.02f) + lum * (grainHighlight * 0.02f);
        float noise = (rand01(nseed) - 0.5f) * (grainAmount * 0.004f) * tonalWeight * (1.0f - grainSoftness * 0.004f);

        if (grainMode == 0) { // Monochrome
          r = clamp01(r + noise * (1.0f + grainLuma * 0.01f));
          g = clamp01(g + noise * (1.0f + grainLuma * 0.01f));
          b = clamp01(b + noise * (1.0f + grainLuma * 0.01f));
        } else { // Color Organic / RF Hiss
          float noiseR = (rand01(nseed + 1) - 0.5f) * (grainAmount * 0.004f) * tonalWeight * (1.0f - grainSoftness * 0.004f);
          float noiseB = (rand01(nseed + 2) - 0.5f) * (grainAmount * 0.004f) * tonalWeight * (1.0f - grainSoftness * 0.004f);
          r = clamp01(r + noiseR * (1.0f + grainChroma * 0.01f));
          g = clamp01(g + noise * (1.0f + grainLuma * 0.01f));
          b = clamp01(b + noiseB * (1.0f + grainChroma * 0.01f));
        }
      }

      // 4. CRT TV Scanlines, Phosphor Bloom & 60Hz Flicker
      if (crtEnabled) {
        float scanWave = 0.5f + 0.5f * std::cos((float)y * 3.14159265f);
        float scanGain = 1.0f - (crtScanlines * 0.003f) * scanWave;

        // 60Hz CRT Flicker
        if (crtFlicker > 0.001f) {
          scanGain *= 1.0f - crtFlicker * 0.0015f * std::sin(timeSec * 377.0f);
        }

        float lum = 0.2126f * r + 0.7152f * g + 0.0722f * b;
        float halo = std::max(0.0f, lum - 0.65f) * (crtBloom * 0.002f);

        if (crtRgb > 0.001f) {
          int off = clampi((int)std::round(crtRgb * 0.03f), 0, 3);
          PF_Pixel rp = inBase[sy * inStride + clampi(sx + off, 0, inW - 1)];
          PF_Pixel bp = inBase[sy * inStride + clampi(sx - off, 0, inW - 1)];
          r = rp.red / 255.0f;
          b = bp.blue / 255.0f;
        }
        r = clamp01(r * scanGain + halo);
        g = clamp01(g * scanGain + halo * 0.9f);
        b = clamp01(b * scanGain + halo * 1.1f);
      }

      // 7. Layer Overlay Sampling & Compositing with Speed/Scale/Loop Period
      if (damageEnabled && overlayWorld && overlayWorld->data && damageOpacity > 0.001f) {
        float oTimeSec = timeSec * (damageSpeed * 0.01f);
        if (damageLoop > 0.001f) {
          oTimeSec = std::fmod(oTimeSec, damageLoop);
        }
        float oScaleFactor = std::max(0.1f, damageScale * 0.01f);
        int ox = clampi((int)(((x + oTimeSec * 20.0f) / oScaleFactor) * overlayWorld->width / std::max(1, w)), 0, overlayWorld->width - 1);
        int oy = clampi((int)((y / oScaleFactor) * overlayWorld->height / std::max(1, h)), 0, overlayWorld->height - 1);
        int oStride = overlayWorld->rowbytes / (int)sizeof(PF_Pixel);
        PF_Pixel op = ((const PF_Pixel*)overlayWorld->data)[oy * oStride + ox];
        float oA = (damageOpacity * 0.01f) * (op.alpha / 255.0f);
        float oR = op.red / 255.0f, oG = op.green / 255.0f, oB = op.blue / 255.0f;

        if (damageBlend == 1) { // Blend
          r = r * (1.0f - oA) + oR * oA;
          g = g * (1.0f - oA) + oG * oA;
          b = b * (1.0f - oA) + oB * oA;
        } else if (damageBlend == 2) { // Add
          r = clamp01(r + oR * oA);
          g = clamp01(g + oG * oA);
          b = clamp01(b + oB * oA);
        } else if (damageBlend == 3) { // Screen
          r = 1.0f - (1.0f - r) * (1.0f - oR * oA);
          g = 1.0f - (1.0f - g) * (1.0f - oG * oA);
          b = 1.0f - (1.0f - b) * (1.0f - oB * oA);
        } else { // Normal
          r = r * (1.0f - oA) + oR * oA;
          g = g * (1.0f - oA) + oG * oA;
          b = b * (1.0f - oA) + oB * oA;
        }
      }

      dst[x].red = (uint8_t)std::round(clamp01(r) * 255.0f);
      dst[x].green = (uint8_t)std::round(clamp01(g) * 255.0f);
      dst[x].blue = (uint8_t)std::round(clamp01(b) * 255.0f);
      dst[x].alpha = p.alpha;
    }
  }

  // Render Date & Text OSD Overlays
  const bool textEnabled = getCheck(params, VHS_TEXT_ENABLE);
  if (textEnabled) {
    const int textStyle = params[VHS_TEXT_STYLE]->u.pd.value;
    const int year = getInt(params, VHS_TEXT_YEAR);
    const int month = getInt(params, VHS_TEXT_MONTH);
    const int day = getInt(params, VHS_TEXT_DAY);
    const int hour = getInt(params, VHS_TEXT_HOUR);
    const int minute = getInt(params, VHS_TEXT_MINUTE);
    const int presetStrIdx = params[VHS_TEXT_PRESET_STRING]->u.pd.value;
    const float posX = getSlider(params, VHS_TEXT_POS_X, 100.0f);
    const float posY = getSlider(params, VHS_TEXT_POS_Y, 100.0f);
    const float textScale = getSlider(params, VHS_TEXT_SCALE, 100.0f) * 2.0f;
    const float textOpacity = getSlider(params, VHS_TEXT_OPACITY, 100.0f);

    int startX = (int)(posX * w);
    int startY = (int)(posY * h);

    char osdBuffer[128];
    const char *presetStrings[] = { "PLAY", "REC ●", "PAUSE", "AUTO SP", "STOP", "VHS MASTER" };
    const char *selPreset = presetStrings[clampi(presetStrIdx - 1, 0, 5)];

    if (textStyle == 1) { // Date & Time
      snprintf(osdBuffer, sizeof(osdBuffer), "%02d/%02d/%04d\n%02d:%02d:00", month, day, year, hour, minute);
    } else if (textStyle == 2) { // PLAY + Date
      snprintf(osdBuffer, sizeof(osdBuffer), "PLAY ▶\n%02d/%02d/%04d", month, day, year);
    } else if (textStyle == 3) { // REC + Timestamp
      snprintf(osdBuffer, sizeof(osdBuffer), "REC ●  0:00:12\n%02d:%02d", hour, minute);
    } else if (textStyle == 4) { // AUTO SP
      snprintf(osdBuffer, sizeof(osdBuffer), "AUTO SP\n%02d/%02d/%04d", month, day, year);
    } else { // Custom Text Preset
      snprintf(osdBuffer, sizeof(osdBuffer), "%s\n%02d/%02d/%04d", selPreset, month, day, year);
    }

    DrawOSDText(outBase, w, h, outStride, osdBuffer, startX, startY, textScale, textOpacity);
  }

  if (overlayWorld) {
    PF_CHECKIN_PARAM(in_data, &overlayCheckout);
  }

  return err;
}

static PF_Err AudioRender(PF_InData *in_data, PF_OutData *out_data, PF_ParamDef *params[]) {
  PF_SoundWorld *src = (PF_SoundWorld*)&in_data->src_snd;
  PF_SoundWorld *dst = (PF_SoundWorld*)&out_data->dest_snd;
  if (!src->dataP || !dst->dataP || src->num_samples <= 0 || dst->num_samples <= 0) return PF_Err_NONE;

  const A_long srcChannels = std::max<A_long>(1, std::min<A_long>(2, src->fi.num_channels));
  const A_long dstChannels = std::max<A_long>(1, std::min<A_long>(2, dst->fi.num_channels));
  const A_long frames = std::min(src->num_samples, dst->num_samples);
  const bool enabled = getCheck(params, VHS_AUDIO_ENABLE);
  const bool audioEvents = getCheck(params, VHS_AUDIO_EVENT_ENABLE);
  const float mix = getSlider(params, VHS_AUDIO_WET_DRY, 100.0f);
  const float outputGain = getSlider(params, VHS_AUDIO_OUTPUT_GAIN, 1.0f);
  const float distortion = getSlider(params, VHS_AUDIO_DISTORTION, 100.0f);
  const float bitDepth = getSlider(params, VHS_AUDIO_BIT_DEPTH, 100.0f);
  const float sampleRateRed = getSlider(params, VHS_AUDIO_SAMPLE_RATE, 100.0f);
  const float crackle = getSlider(params, VHS_AUDIO_CRACKLE, 100.0f);
  const float dropouts = getSlider(params, VHS_AUDIO_DROPOUTS, 100.0f);
  const float flutter = getSlider(params, VHS_AUDIO_FLUTTER, 100.0f);
  const int audioSeed = getInt(params, VHS_AUDIO_SEED);

  const float *inSamples = (const float*)src->dataP;
  float *outSamples = (float*)dst->dataP;

  for (A_long i = 0; i < frames; ++i) {
    for (A_long ch = 0; ch < dstChannels; ++ch) {
      A_long srcIdx = i * srcChannels + std::min(ch, srcChannels - 1);
      A_long dstIdx = i * dstChannels + ch;
      float dry = inSamples[srcIdx];
      float wet = dry;

      // Sample Rate Reduction & Flutter Jitter
      if (sampleRateRed > 0.001f || flutter > 0.001f) {
        float fPhase = std::sin(i * 0.01f + flutter * 0.1f);
        wet += fPhase * flutter * 0.001f;
      }

      // Crackle / Static Noise
      if (crackle > 0.001f) {
        uint32_t cHash = hash32((uint32_t)i ^ (uint32_t)ch ^ (uint32_t)audioSeed);
        if (rand01(cHash) < (crackle * 0.01f)) {
          wet += (rand01(cHash + 1) - 0.5f) * crackle * 0.05f;
        }
      }

      // Audio Dropouts
      if (dropouts > 0.001f) {
        uint32_t dHash = hash32((uint32_t)(i / 128) ^ (uint32_t)audioSeed);
        if (rand01(dHash) < (dropouts * 0.005f)) {
          wet *= 0.10f;
        }
      }

      // Tape Saturation Distortion
      if (distortion > 0.0f) {
        float drive = 1.0f + distortion * 5.0f;
        wet = std::tanh(wet * drive) / std::tanh(drive);
      }

      // Bit Depth Quantization
      if (bitDepth > 0.001f) {
        float levels = std::pow(2.0f, std::max(2.0f, 16.0f - bitDepth * 0.12f));
        wet = std::round(wet * levels) / levels;
      }

      outSamples[dstIdx] = enabled ? dry * (1.0f - mix) + wet * mix * std::pow(10.0f, outputGain / 20.0f) : dry;
    }
  }
  return PF_Err_NONE;
}

extern "C" DllExport PF_Err EffectMain(PF_Cmd cmd, PF_InData *in_data, PF_OutData *out_data, PF_ParamDef *params[], PF_LayerDef *output, void *) {
  PF_Err err = PF_Err_NONE;
  switch (cmd) {
    case PF_Cmd_ABOUT: err = About(out_data); break;
    case PF_Cmd_GLOBAL_SETUP: err = GlobalSetup(out_data); break;
    case PF_Cmd_PARAMS_SETUP: err = ParamsSetup(in_data, out_data); break;
    case PF_Cmd_RENDER: err = Render(in_data, params, output); break;
    case PF_Cmd_AUDIO_RENDER: err = AudioRender(in_data, out_data, params); break;
    default: break;
  }
  return err;
}
