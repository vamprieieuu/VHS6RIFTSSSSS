#include "AEConfig.h"
#ifdef AE_OS_WIN
  #include <Windows.h>
#endif
// The supplied SDK snapshot does not include entry.h; EffectMain is the explicit
// exported entry point for this module, so define the export decoration locally.
#ifndef DllExport
  #define DllExport __declspec(dllexport)
#endif
#include "AE_Effect.h"
#include "AE_EffectCB.h"
#include "AE_Macros.h"
#include "Param_Utils.h"
#include "String_Utils.h"
#include <cmath>
#include <cstdint>
#include <algorithm>

#define PLUGIN_NAME "VHS6RIFT"
#define PLUGIN_MATCH_NAME "com.vhs6rift.effect"
#define PLUGIN_VERSION 1

// Stable parameter IDs. Topic parameters are included in sequence; do not reorder without updating IDs.
enum {
  P_INPUT = 0, P_CAMERA, P_MASTER, P_CONTINUOUS, P_EVENTS, P_SEED,
  P_TOPIC_CAMERA,
  P_FXC6_TOPIC, P_FXC6_CHARACTER, P_FXC6_COLOR, P_FXC6_SIGNAL, P_FXC6_GRAIN, P_FXC6_END,
  P_GEN7_TOPIC, P_GEN7_CHARACTER, P_GEN7_COLOR, P_GEN7_SIGNAL, P_GEN7_GRAIN, P_GEN7_END,
  P_JVC_C7_TOPIC, P_JVC_C7_CHARACTER, P_JVC_C7_COLOR, P_JVC_C7_SIGNAL, P_JVC_C7_GRAIN, P_JVC_C7_END,
  P_R20_TOPIC, P_R20_CHARACTER, P_R20_COLOR, P_R20_SIGNAL, P_R20_GRAIN, P_R20_END,
  P_FALL78_TOPIC, P_FALL78_CHARACTER, P_FALL78_COLOR, P_FALL78_SIGNAL, P_FALL78_GRAIN, P_FALL78_END,
  P_ERC_TOPIC, P_ERC_CHARACTER, P_ERC_COLOR, P_ERC_SIGNAL, P_ERC_GRAIN, P_ERC_END,
  P_JVC_SXM_TOPIC, P_JVC_SXM_CHARACTER, P_JVC_SXM_COLOR, P_JVC_SXM_SIGNAL, P_JVC_SXM_GRAIN, P_JVC_SXM_END,
  P_TOPIC_CAMERA_END,
  P_TOPIC_COLOR, P_SATURATION, P_BLACK, P_SOFTNESS, P_CHROMA_BLEED, P_CHROMA_X, P_CHROMA_Y, P_TOPIC_COLOR_END,
  P_TOPIC_GRAIN, P_GRAIN_AMOUNT, P_GRAIN_SCALE, P_GRAIN_SPEED, P_TOPIC_GRAIN_END,
  P_TOPIC_DAMAGE, P_TRACKING, P_TRACK_FREQ, P_TRACK_WIDTH, P_DROPOUT, P_BLOCK_GLITCH, P_EVENT_RATE, P_EVENT_DURATION, P_RECOVERY, P_TOPIC_DAMAGE_END,
  P_TOPIC_TAPE, P_TAPE_TYPE, P_TAPE_STRENGTH, P_TAPE_SIZE, P_TAPE_SPEED, P_TAPE_DIRECTION, P_TOPIC_TAPE_END,
  P_TOPIC_OVERLAY, P_OVERLAY_LAYER, P_OVERLAY_MODE, P_OVERLAY_OPACITY, P_OVERLAY_SPEED, P_OVERLAY_OFFSET, P_OVERLAY_LOOP, P_OVERLAY_LOOP_PERIOD, P_TOPIC_OVERLAY_END,
  P_TOPIC_OTHER, P_OTHER_ENGINE, P_OTHER_STRENGTH,
  P_CRAFT_TOPIC, P_CRAFT_TRACKING, P_CRAFT_CHROMA, P_CRAFT_HEADSWITCH, P_CRAFT_SMEAR, P_CRAFT_END,
  P_LEGEND_TOPIC, P_LEGEND_BREAKUP, P_LEGEND_TEAR, P_LEGEND_SIGNAL, P_LEGEND_BLOCKS, P_LEGEND_END,
  P_NTSC_TOPIC, P_NTSC_CHROMA_DELAY, P_NTSC_HEAD_SWITCH, P_NTSC_EDGE_WAVE, P_NTSC_COMPOSITE, P_NTSC_END,
  P_SIGNAL_TOPIC, P_SIGNAL_STRENGTH, P_SIGNAL_LUMA_NOISE, P_SIGNAL_CHROMA_NOISE, P_SIGNAL_TAPE_ERRORS, P_SIGNAL_END,
  P_TOPIC_OTHER_END,
  P_TOPIC_AUDIO, P_AUDIO_ENABLE, P_AUDIO_EVENTS, P_AUDIO_MIX, P_AUDIO_EVENT_AMOUNT, P_AUDIO_CRACKLE, P_AUDIO_DROPOUT, P_AUDIO_SATURATION, P_AUDIO_LOFI, P_TOPIC_AUDIO_END,
  P_TOPIC_MODULES, P_GRAIN_ENABLE, P_TRACKING_ENABLE, P_TAPE_ENABLE, P_OVERLAY_ENABLE, P_OTHER_ENABLE,
  P_CRT_ENABLE, P_CRT_SCANLINES, P_CRT_CURVE, P_CRT_GLOW, P_CRT_RGB, P_CRT_FLICKER, P_CRT_VIGNETTE, P_TOPIC_MODULES_END,
  P_TOPIC_ADV_TAPE, P_TRANSPARENT_TAPE_ENABLE, P_TRANSPARENT_TAPE_STRENGTH, P_TRANSPARENT_TAPE_WIDTH, P_TRANSPARENT_TAPE_SPEED,
  P_TAPE_SWEEP_ENABLE, P_TAPE_SWEEP_STRENGTH, P_TAPE_SWEEP_WIDTH, P_TAPE_SWEEP_SPEED, P_TAPE_SWEEP_INTERVAL, P_TOPIC_ADV_TAPE_END,
  P_COUNT
};
static PF_Err About(PF_InData *in_data, PF_OutData *out_data, PF_ParamDef *params[], PF_LayerDef *output) {
  PF_SPRINTF(out_data->return_msg, "%s 1.2 | Native VHS video processing for After Effects 23.x", PLUGIN_NAME);
  return PF_Err_NONE;
}

static PF_Err GlobalSetup(PF_OutData *out_data) {
  out_data->my_version = PF_VERSION(1, 2, 0, PF_Stage_RELEASE, 1);
  out_data->out_flags = PF_OutFlag_WIDE_TIME_INPUT | PF_OutFlag_NON_PARAM_VARY | PF_OutFlag_AUDIO_EFFECT_TOO | PF_OutFlag_AUDIO_FLOAT_ONLY;
  out_data->out_flags2 = 0;
  return PF_Err_NONE;
}

static PF_Err ParamsSetup(PF_InData *in_data, PF_OutData *out_data) {
  PF_Err err = PF_Err_NONE;
  PF_ParamDef def;

  AEFX_CLR_STRUCT(def);

  PF_ADD_POPUP("VHS Camcorder / Reference Profile", 7, 1,
    "Fxc6 Reference|Gen 7 Reference|JVC GR-C7U (1986)|VHS-R20 Reference|Fall-78 Reference|ERC-V2S Reference|JVC GR-SXM740U", P_CAMERA);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Master VHS Strength", 0, 2000, 0, 2000, 350, PF_Precision_TENTHS, 0, 0, P_MASTER);
  AEFX_CLR_STRUCT(def);
  PF_ADD_CHECKBOX("Continuous VHS Treatment", "Enable", TRUE, 0, P_CONTINUOUS);
  AEFX_CLR_STRUCT(def);
  PF_ADD_CHECKBOX("Event Glitches", "Enable", FALSE, 0, P_EVENTS);
  AEFX_CLR_STRUCT(def);
  PF_ADD_SLIDER("Random Seed", 0, 9999, 0, 9999, 17, P_SEED);

  AEFX_CLR_STRUCT(def);

  PF_ADD_TOPIC("Camera-Specific Controls", P_TOPIC_CAMERA);
  AEFX_CLR_STRUCT(def);
  PF_ADD_TOPIC("Fxc6 Reference Profile", P_FXC6_TOPIC);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Fxc6 Character", 0, 100, 0, 100, 65, PF_Precision_TENTHS, 0, 0, P_FXC6_CHARACTER);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Fxc6 Color Response", 0, 100, 0, 100, 55, PF_Precision_TENTHS, 0, 0, P_FXC6_COLOR);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Fxc6 Signal Damage", 0, 100, 0, 100, 35, PF_Precision_TENTHS, 0, 0, P_FXC6_SIGNAL);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Fxc6 Grain", 0, 100, 0, 100, 28, PF_Precision_TENTHS, 0, 0, P_FXC6_GRAIN);
  AEFX_CLR_STRUCT(def);
  PF_END_TOPIC(P_FXC6_END);
  AEFX_CLR_STRUCT(def);
  PF_ADD_TOPIC("Gen 7 Reference Profile", P_GEN7_TOPIC);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Gen 7 Character", 0, 100, 0, 100, 75, PF_Precision_TENTHS, 0, 0, P_GEN7_CHARACTER);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Gen 7 Color Response", 0, 100, 0, 100, 80, PF_Precision_TENTHS, 0, 0, P_GEN7_COLOR);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Gen 7 Signal Damage", 0, 100, 0, 100, 85, PF_Precision_TENTHS, 0, 0, P_GEN7_SIGNAL);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Gen 7 Grain", 0, 100, 0, 100, 70, PF_Precision_TENTHS, 0, 0, P_GEN7_GRAIN);
  AEFX_CLR_STRUCT(def);
  PF_END_TOPIC(P_GEN7_END);
  AEFX_CLR_STRUCT(def);
  PF_ADD_TOPIC("JVC GR-C7U (1986) Profile", P_JVC_C7_TOPIC);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("JVC GR-C7U Character", 0, 100, 0, 100, 82, PF_Precision_TENTHS, 0, 0, P_JVC_C7_CHARACTER);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("JVC GR-C7U Color Response", 0, 100, 0, 100, 65, PF_Precision_TENTHS, 0, 0, P_JVC_C7_COLOR);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("JVC GR-C7U Signal Damage", 0, 100, 0, 100, 75, PF_Precision_TENTHS, 0, 0, P_JVC_C7_SIGNAL);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("JVC GR-C7U Grain", 0, 100, 0, 100, 60, PF_Precision_TENTHS, 0, 0, P_JVC_C7_GRAIN);
  AEFX_CLR_STRUCT(def);
  PF_END_TOPIC(P_JVC_C7_END);
  AEFX_CLR_STRUCT(def);
  PF_ADD_TOPIC("VHS-R20 Reference Profile", P_R20_TOPIC);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("VHS-R20 Character", 0, 100, 0, 100, 55, PF_Precision_TENTHS, 0, 0, P_R20_CHARACTER);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("VHS-R20 Color Response", 0, 100, 0, 100, 70, PF_Precision_TENTHS, 0, 0, P_R20_COLOR);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("VHS-R20 Signal Damage", 0, 100, 0, 100, 50, PF_Precision_TENTHS, 0, 0, P_R20_SIGNAL);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("VHS-R20 Grain", 0, 100, 0, 100, 30, PF_Precision_TENTHS, 0, 0, P_R20_GRAIN);
  AEFX_CLR_STRUCT(def);
  PF_END_TOPIC(P_R20_END);
  AEFX_CLR_STRUCT(def);
  PF_ADD_TOPIC("Fall-78 Reference Profile", P_FALL78_TOPIC);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Fall-78 Character", 0, 100, 0, 100, 30, PF_Precision_TENTHS, 0, 0, P_FALL78_CHARACTER);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Fall-78 Color Response", 0, 100, 0, 100, 45, PF_Precision_TENTHS, 0, 0, P_FALL78_COLOR);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Fall-78 Signal Damage", 0, 100, 0, 100, 25, PF_Precision_TENTHS, 0, 0, P_FALL78_SIGNAL);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Fall-78 Grain", 0, 100, 0, 100, 15, PF_Precision_TENTHS, 0, 0, P_FALL78_GRAIN);
  AEFX_CLR_STRUCT(def);
  PF_END_TOPIC(P_FALL78_END);
  AEFX_CLR_STRUCT(def);
  PF_ADD_TOPIC("ERC-V2S Reference Profile", P_ERC_TOPIC);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("ERC-V2S Character", 0, 100, 0, 100, 60, PF_Precision_TENTHS, 0, 0, P_ERC_CHARACTER);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("ERC-V2S Color Response", 0, 100, 0, 100, 90, PF_Precision_TENTHS, 0, 0, P_ERC_COLOR);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("ERC-V2S Signal Damage", 0, 100, 0, 100, 45, PF_Precision_TENTHS, 0, 0, P_ERC_SIGNAL);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("ERC-V2S Grain", 0, 100, 0, 100, 40, PF_Precision_TENTHS, 0, 0, P_ERC_GRAIN);
  AEFX_CLR_STRUCT(def);
  PF_END_TOPIC(P_ERC_END);
  AEFX_CLR_STRUCT(def);
  PF_ADD_TOPIC("JVC GR-SXM740U Profile", P_JVC_SXM_TOPIC);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("JVC GR-SXM740U Character", 0, 100, 0, 100, 50, PF_Precision_TENTHS, 0, 0, P_JVC_SXM_CHARACTER);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("JVC GR-SXM740U Color Response", 0, 100, 0, 100, 65, PF_Precision_TENTHS, 0, 0, P_JVC_SXM_COLOR);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("JVC GR-SXM740U Signal Damage", 0, 100, 0, 100, 35, PF_Precision_TENTHS, 0, 0, P_JVC_SXM_SIGNAL);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("JVC GR-SXM740U Grain", 0, 100, 0, 100, 25, PF_Precision_TENTHS, 0, 0, P_JVC_SXM_GRAIN);
  AEFX_CLR_STRUCT(def);
  PF_END_TOPIC(P_JVC_SXM_END);
  AEFX_CLR_STRUCT(def);
  PF_END_TOPIC(P_TOPIC_CAMERA_END);

  AEFX_CLR_STRUCT(def);

  PF_ADD_TOPIC("Color Controls", P_TOPIC_COLOR);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Saturation", 0, 200, 0, 200, 100, PF_Precision_TENTHS, 0, 0, P_SATURATION);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Black Level", -100, 100, -100, 100, 0, PF_Precision_TENTHS, 0, 0, P_BLACK);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Luma Softness", 0, 100, 0, 100, 28, PF_Precision_TENTHS, 0, 0, P_SOFTNESS);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Chroma Bleed", 0, 100, 0, 100, 42, PF_Precision_TENTHS, 0, 0, P_CHROMA_BLEED);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Chroma Offset X", -30, 30, -30, 30, 4, PF_Precision_TENTHS, 0, 0, P_CHROMA_X);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Chroma Offset Y", -20, 20, -20, 20, 0, PF_Precision_TENTHS, 0, 0, P_CHROMA_Y);
  AEFX_CLR_STRUCT(def);
  PF_END_TOPIC(P_TOPIC_COLOR_END);

  AEFX_CLR_STRUCT(def);

  PF_ADD_TOPIC("Grain and Signal Noise", P_TOPIC_GRAIN);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Colored Grain Amount", 0, 100, 0, 100, 0, PF_Precision_TENTHS, 0, 0, P_GRAIN_AMOUNT);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Grain Scale", 1, 100, 1, 100, 28, PF_Precision_TENTHS, 0, 0, P_GRAIN_SCALE);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Grain Motion Speed", 0, 400, 0, 400, 100, PF_Precision_TENTHS, 0, 0, P_GRAIN_SPEED);
  AEFX_CLR_STRUCT(def);
  PF_END_TOPIC(P_TOPIC_GRAIN_END);

  AEFX_CLR_STRUCT(def);

  PF_ADD_TOPIC("Tracking and Damage Events", P_TOPIC_DAMAGE);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Tracking Noise", 0, 100, 0, 100, 0, PF_Precision_TENTHS, 0, 0, P_TRACKING);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Tracking Event Frequency", 0, 100, 0, 100, 25, PF_Precision_TENTHS, 0, 0, P_TRACK_FREQ);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Tracking Band Width", 1, 100, 1, 100, 18, PF_Precision_TENTHS, 0, 0, P_TRACK_WIDTH);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Tape Dropouts", 0, 100, 0, 100, 0, PF_Precision_TENTHS, 0, 0, P_DROPOUT);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Block Corruption", 0, 100, 0, 100, 0, PF_Precision_TENTHS, 0, 0, P_BLOCK_GLITCH);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Events per Minute", 0, 60, 0, 60, 8, PF_Precision_TENTHS, 0, 0, P_EVENT_RATE);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Event Duration (seconds)", 0.02, 2.0, 0.02, 2.0, 0.18, PF_Precision_HUNDREDTHS, 0, 0, P_EVENT_DURATION);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Recovery (seconds)", 0, 2.0, 0, 2.0, 0.12, PF_Precision_HUNDREDTHS, 0, 0, P_RECOVERY);
  AEFX_CLR_STRUCT(def);
  PF_END_TOPIC(P_TOPIC_DAMAGE_END);

  AEFX_CLR_STRUCT(def);

  PF_ADD_TOPIC("Tape Band / Tape Noise", P_TOPIC_TAPE);
  AEFX_CLR_STRUCT(def);
  PF_ADD_POPUP("Tape Type", 6, 1, "Off|Horizontal Band|Bottom Head-Switch|Vertical Roll|Side Tear|Signal Snow", P_TAPE_TYPE);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Tape Strength", 0, 100, 0, 100, 35, PF_Precision_TENTHS, 0, 0, P_TAPE_STRENGTH);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Tape Size", 1, 100, 1, 100, 18, PF_Precision_TENTHS, 0, 0, P_TAPE_SIZE);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Tape Speed", 0, 300, 0, 300, 60, PF_Precision_TENTHS, 0, 0, P_TAPE_SPEED);
  AEFX_CLR_STRUCT(def);
  PF_ADD_POPUP("Tape Direction", 4, 1, "Top to Bottom|Bottom to Top|Left to Right|Right to Left", P_TAPE_DIRECTION);
  AEFX_CLR_STRUCT(def);
  PF_END_TOPIC(P_TOPIC_TAPE_END);

  AEFX_CLR_STRUCT(def);

  PF_ADD_TOPIC("Damage Overlay Layer", P_TOPIC_OVERLAY);
  AEFX_CLR_STRUCT(def);
  PF_ADD_LAYER("Overlay Layer (optional)", PF_LayerDefault_NONE, P_OVERLAY_LAYER);
  AEFX_CLR_STRUCT(def);
  PF_ADD_POPUP("Overlay Display Mode", 6, 1, "Normal|Blend|Add|Screen|Multiply|Luma Key", P_OVERLAY_MODE);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Overlay Opacity", 0, 100, 0, 100, 60, PF_Precision_TENTHS, 0, 0, P_OVERLAY_OPACITY);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Overlay Speed", 0, 400, 0, 400, 100, PF_Precision_TENTHS, 0, 0, P_OVERLAY_SPEED);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Overlay Time Offset (seconds)", -60, 60, -60, 60, 0, PF_Precision_TENTHS, 0, 0, P_OVERLAY_OFFSET);
  AEFX_CLR_STRUCT(def);
  PF_ADD_CHECKBOX("Loop Overlay", "Enable", FALSE, 0, P_OVERLAY_LOOP);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Overlay Loop Period (seconds)", 0.1, 120, 0.1, 120, 2, PF_Precision_TENTHS, 0, 0, P_OVERLAY_LOOP_PERIOD);
  AEFX_CLR_STRUCT(def);
  PF_END_TOPIC(P_TOPIC_OVERLAY_END);

  AEFX_CLR_STRUCT(def);
  PF_ADD_TOPIC("Other VHS Engines (independent implementations)", P_TOPIC_OTHER);
  AEFX_CLR_STRUCT(def);
  PF_ADD_POPUP("Other VHS Engine", 5, 1, "Off|VHS Craft Reference|VHS Legend Reference File|NTSC-RS Signal Model|Signal Reference", P_OTHER_ENGINE);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Other Engine Strength", 0, 100, 0, 100, 50, PF_Precision_TENTHS, 0, 0, P_OTHER_STRENGTH);
  AEFX_CLR_STRUCT(def);
  PF_ADD_TOPIC("VHS Craft Reference Controls", P_CRAFT_TOPIC);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Craft Tracking Interference", 0, 100, 0, 100, 25, PF_Precision_TENTHS, 0, 0, P_CRAFT_TRACKING);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Craft Chroma Smear", 0, 100, 0, 100, 30, PF_Precision_TENTHS, 0, 0, P_CRAFT_CHROMA);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Craft Head-Switch Spikes", 0, 100, 0, 100, 20, PF_Precision_TENTHS, 0, 0, P_CRAFT_HEADSWITCH);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Craft Signal Blur", 0, 100, 0, 100, 15, PF_Precision_TENTHS, 0, 0, P_CRAFT_SMEAR);
  AEFX_CLR_STRUCT(def);
  PF_END_TOPIC(P_CRAFT_END);
  AEFX_CLR_STRUCT(def);
  PF_ADD_TOPIC("VHS Legend Reference Controls", P_LEGEND_TOPIC);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Legend Signal Breakdown", 0, 100, 0, 100, 25, PF_Precision_TENTHS, 0, 0, P_LEGEND_BREAKUP);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Legend Horizontal Tear", 0, 100, 0, 100, 20, PF_Precision_TENTHS, 0, 0, P_LEGEND_TEAR);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Legend Signal Loss", 0, 100, 0, 100, 10, PF_Precision_TENTHS, 0, 0, P_LEGEND_SIGNAL);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Legend Block Corruption", 0, 100, 0, 100, 15, PF_Precision_TENTHS, 0, 0, P_LEGEND_BLOCKS);
  AEFX_CLR_STRUCT(def);
  PF_END_TOPIC(P_LEGEND_END);
  AEFX_CLR_STRUCT(def);
  PF_ADD_TOPIC("NTSC-RS Reference Controls", P_NTSC_TOPIC);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("NTSC Chroma Delay", 0, 100, 0, 100, 25, PF_Precision_TENTHS, 0, 0, P_NTSC_CHROMA_DELAY);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("NTSC Head Switching", 0, 100, 0, 100, 18, PF_Precision_TENTHS, 0, 0, P_NTSC_HEAD_SWITCH);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("NTSC Edge Wave", 0, 100, 0, 100, 15, PF_Precision_TENTHS, 0, 0, P_NTSC_EDGE_WAVE);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("NTSC Composite Signal", 0, 100, 0, 100, 35, PF_Precision_TENTHS, 0, 0, P_NTSC_COMPOSITE);
  AEFX_CLR_STRUCT(def);
  PF_END_TOPIC(P_NTSC_END);
  AEFX_CLR_STRUCT(def);
  PF_ADD_TOPIC("Signal Reference Controls", P_SIGNAL_TOPIC);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Signal Strength", 0, 100, 0, 100, 35, PF_Precision_TENTHS, 0, 0, P_SIGNAL_STRENGTH);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Signal Luma Noise", 0, 100, 0, 100, 20, PF_Precision_TENTHS, 0, 0, P_SIGNAL_LUMA_NOISE);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Signal Chroma Noise", 0, 100, 0, 100, 25, PF_Precision_TENTHS, 0, 0, P_SIGNAL_CHROMA_NOISE);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Signal Tape Errors", 0, 100, 0, 100, 20, PF_Precision_TENTHS, 0, 0, P_SIGNAL_TAPE_ERRORS);
  AEFX_CLR_STRUCT(def);
  PF_END_TOPIC(P_SIGNAL_END);
  AEFX_CLR_STRUCT(def);
  PF_END_TOPIC(P_TOPIC_OTHER_END);

  AEFX_CLR_STRUCT(def);
  PF_ADD_TOPIC("Audio Glitches (real audio processing)", P_TOPIC_AUDIO);
  AEFX_CLR_STRUCT(def);
  PF_ADD_CHECKBOX("Audio Glitch Enable", "Enable", FALSE, 0, P_AUDIO_ENABLE);
  AEFX_CLR_STRUCT(def);
  PF_ADD_CHECKBOX("Audio Event Glitches", "Enable", FALSE, 0, P_AUDIO_EVENTS);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Audio Wet / Dry Mix", 0, 100, 0, 100, 45, PF_Precision_TENTHS, 0, 0, P_AUDIO_MIX);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Audio Event Amount", 0, 100, 0, 100, 55, PF_Precision_TENTHS, 0, 0, P_AUDIO_EVENT_AMOUNT);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Audio Crackle / Static", 0, 100, 0, 100, 18, PF_Precision_TENTHS, 0, 0, P_AUDIO_CRACKLE);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Audio Dropouts", 0, 100, 0, 100, 10, PF_Precision_TENTHS, 0, 0, P_AUDIO_DROPOUT);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Audio Saturation", 0, 100, 0, 100, 12, PF_Precision_TENTHS, 0, 0, P_AUDIO_SATURATION);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Lo-Fi Quantization", 0, 100, 0, 100, 8, PF_Precision_TENTHS, 0, 0, P_AUDIO_LOFI);
  AEFX_CLR_STRUCT(def);
  PF_END_TOPIC(P_TOPIC_AUDIO_END);

  AEFX_CLR_STRUCT(def);
  PF_ADD_TOPIC("Independent Effect Modules", P_TOPIC_MODULES);
  AEFX_CLR_STRUCT(def);
  PF_ADD_CHECKBOX("Enable Grain / Noise", "Enable", FALSE, 0, P_GRAIN_ENABLE);
  AEFX_CLR_STRUCT(def);
  PF_ADD_CHECKBOX("Enable Tracking Damage", "Enable", FALSE, 0, P_TRACKING_ENABLE);
  AEFX_CLR_STRUCT(def);
  PF_ADD_CHECKBOX("Enable Tape Effects", "Enable", FALSE, 0, P_TAPE_ENABLE);
  AEFX_CLR_STRUCT(def);
  PF_ADD_CHECKBOX("Enable Overlay Layer", "Enable", FALSE, 0, P_OVERLAY_ENABLE);
  AEFX_CLR_STRUCT(def);
  PF_ADD_CHECKBOX("Enable Other Signal Engines", "Enable", FALSE, 0, P_OTHER_ENABLE);
  AEFX_CLR_STRUCT(def);
  PF_ADD_CHECKBOX("Enable CRT TV Engine", "Enable", FALSE, 0, P_CRT_ENABLE);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("CRT Scanline Strength", 0, 100, 0, 100, 35, PF_Precision_TENTHS, 0, 0, P_CRT_SCANLINES);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("CRT Curvature", 0, 100, 0, 100, 20, PF_Precision_TENTHS, 0, 0, P_CRT_CURVE);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("CRT Glow", 0, 100, 0, 100, 25, PF_Precision_TENTHS, 0, 0, P_CRT_GLOW);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("CRT RGB Convergence", 0, 100, 0, 100, 18, PF_Precision_TENTHS, 0, 0, P_CRT_RGB);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("CRT Flicker", 0, 100, 0, 100, 8, PF_Precision_TENTHS, 0, 0, P_CRT_FLICKER);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("CRT Edge Vignette", 0, 100, 0, 100, 28, PF_Precision_TENTHS, 0, 0, P_CRT_VIGNETTE);
  AEFX_CLR_STRUCT(def);
  PF_END_TOPIC(P_TOPIC_MODULES_END);

  AEFX_CLR_STRUCT(def);
  PF_ADD_TOPIC("Transparent Tape and Tape Sweep", P_TOPIC_ADV_TAPE);
  AEFX_CLR_STRUCT(def);
  PF_ADD_CHECKBOX("Procedural Transparent Tape", "Enable", FALSE, 0, P_TRANSPARENT_TAPE_ENABLE);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Transparent Tape Strength", 0, 100, 0, 100, 35, PF_Precision_TENTHS, 0, 0, P_TRANSPARENT_TAPE_STRENGTH);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Transparent Tape Width", 1, 100, 1, 100, 30, PF_Precision_TENTHS, 0, 0, P_TRANSPARENT_TAPE_WIDTH);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Transparent Tape Speed", 0, 300, 0, 300, 45, PF_Precision_TENTHS, 0, 0, P_TRANSPARENT_TAPE_SPEED);
  AEFX_CLR_STRUCT(def);
  PF_ADD_CHECKBOX("Moving Tape Sweep", "Enable", FALSE, 0, P_TAPE_SWEEP_ENABLE);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Tape Sweep Strength", 0, 100, 0, 100, 50, PF_Precision_TENTHS, 0, 0, P_TAPE_SWEEP_STRENGTH);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Tape Sweep Width", 1, 100, 1, 100, 20, PF_Precision_TENTHS, 0, 0, P_TAPE_SWEEP_WIDTH);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Tape Sweep Speed", 1, 300, 1, 300, 80, PF_Precision_TENTHS, 0, 0, P_TAPE_SWEEP_SPEED);
  AEFX_CLR_STRUCT(def);
  PF_ADD_FLOAT_SLIDERX("Tape Sweep Interval (seconds)", 0.1, 30, 0.1, 30, 5, PF_Precision_TENTHS, 0, 0, P_TAPE_SWEEP_INTERVAL);
  AEFX_CLR_STRUCT(def);
  PF_END_TOPIC(P_TOPIC_ADV_TAPE_END);

  out_data->num_params = P_COUNT;
  return err;
}

static inline float clamp01(float v) { return std::max(0.0f, std::min(1.0f, v)); }
static inline uint32_t hash32(uint32_t x) {
  x ^= x >> 16; x *= 0x7feb352dU; x ^= x >> 15; x *= 0x846ca68bU; x ^= x >> 16; return x;
}
static inline float rand01(uint32_t x) { return (hash32(x) & 0x00ffffffU) / 16777215.0f; }
static inline float smooth(float a, float b, float x) { float t = clamp01((x-a)/(b-a)); return t*t*(3.0f-2.0f*t); }
static inline int clampi(int v, int a, int b) { return std::max(a, std::min(b, v)); }

struct Settings {
  int camera=1, seed=17, tapeType=0, tapeDir=0, overlayMode=0;
  float master=.35f, continuous=1, events=1, saturation=1, black=0, softness=.12f, bleed=.20f;
  float chromaX=2, chromaY=0, grain=.08f, grainScale=28, grainSpeed=1, tracking=.18f, trackFreq=.25f, trackWidth=.18f;
  float dropout=.10f, blocks=.05f, eventRate=8, eventDuration=.18f, recovery=.12f, tapeStrength=.35f, tapeSize=.18f, tapeSpeed=.6f;
};

static float getSlider(PF_ParamDef *params[], int id, float scale) { return (float)params[id]->u.fs_d.value / scale; }
static int getInt(PF_ParamDef *params[], int id) { return params[id]->u.sd.value; }
static bool getCheck(PF_ParamDef *params[], int id) { return params[id]->u.bd.value != 0; }
static Settings ReadSettings(PF_ParamDef *p[]) {
  Settings s;
  s.camera = p[P_CAMERA]->u.pd.value;
  s.master = getSlider(p,P_MASTER,100.0f);
  s.continuous = getCheck(p,P_CONTINUOUS) ? 1.0f : 0.0f;
  s.events = getCheck(p,P_EVENTS) ? 1.0f : 0.0f;
  s.seed = getInt(p,P_SEED);
  s.saturation = getSlider(p,P_SATURATION,100.0f);
  s.black = getSlider(p,P_BLACK,100.0f);
  s.softness = getSlider(p,P_SOFTNESS,100.0f);
  s.bleed = getSlider(p,P_CHROMA_BLEED,100.0f);
  s.chromaX = getSlider(p,P_CHROMA_X,1.0f);
  s.chromaY = getSlider(p,P_CHROMA_Y,1.0f);
  s.grain = getSlider(p,P_GRAIN_AMOUNT,100.0f);
  s.grainScale = getSlider(p,P_GRAIN_SCALE,1.0f);
  s.grainSpeed = getSlider(p,P_GRAIN_SPEED,100.0f);
  s.tracking = getSlider(p,P_TRACKING,100.0f);
  s.trackFreq = getSlider(p,P_TRACK_FREQ,100.0f);
  s.trackWidth = getSlider(p,P_TRACK_WIDTH,100.0f);
  s.dropout = getSlider(p,P_DROPOUT,100.0f);
  s.blocks = getSlider(p,P_BLOCK_GLITCH,100.0f);
  s.eventRate = getSlider(p,P_EVENT_RATE,1.0f);
  s.eventDuration = getSlider(p,P_EVENT_DURATION,1.0f);
  s.recovery = getSlider(p,P_RECOVERY,1.0f);
  s.tapeType = p[P_TAPE_TYPE]->u.pd.value - 1;
  s.tapeStrength = getSlider(p,P_TAPE_STRENGTH,100.0f);
  s.tapeSize = getSlider(p,P_TAPE_SIZE,100.0f);
  s.tapeSpeed = getSlider(p,P_TAPE_SPEED,100.0f);
  s.tapeDir = p[P_TAPE_DIRECTION]->u.pd.value - 1;
  s.overlayMode = p[P_OVERLAY_MODE]->u.pd.value - 1;
  return s;
}

static PF_Err Render(PF_InData *in_data, PF_ParamDef *params[], PF_LayerDef *output) {
  PF_Err err = PF_Err_NONE;
  PF_EffectWorld *src = &params[P_INPUT]->u.ld;
  if (!src || !src->data || !output->data) return PF_Err_NONE;
  Settings s = ReadSettings(params);
  const int w = output->width, h = output->height;
  const int inW = src->width, inH = src->height;
  const PF_Pixel *inBase = (const PF_Pixel*)src->data;
  PF_Pixel *outBase = (PF_Pixel*)output->data;
  const int inStride = src->rowbytes / (int)sizeof(PF_Pixel);
  const int outStride = output->rowbytes / (int)sizeof(PF_Pixel);
  const float timeSec = in_data->time_scale ? (float)in_data->current_time / (float)in_data->time_scale : 0.0f;
  const int frame = (in_data->time_step != 0) ? (int)(in_data->current_time / in_data->time_step) : (int)std::floor(timeSec * 24.0f + 0.5f);
  // 0-2000 master control: preserve a useful low range while allowing extreme stylization.
  // s.master is already divided by 100, therefore 2000 UI units maps to 20.0 here.
  const float strength = std::min(20.0f, std::max(0.0f, s.master));

  // Camera models change the processing path, not merely a LUT.
  static const float cameraSat[7]       = {0.95f, 1.25f, 0.85f, 0.65f, 0.70f, 1.15f, 1.05f};
  static const float cameraSoft[7]      = {0.02f, 0.01f, 0.04f, 0.01f, 0.05f, 0.00f, 0.01f};
  static const float cameraBleed[7]     = {0.20f, 0.15f, 0.35f, 0.10f, 0.40f, 0.08f, 0.18f};
  static const float cameraNoise[7]     = {0.06f, 0.04f, 0.12f, 0.18f, 0.15f, 0.03f, 0.07f};
  static const float cameraWarm[7]      = {0.04f,-0.02f, 0.14f,-0.08f, 0.12f,-0.05f, 0.03f};
  static const float cameraGreen[7]     = {0.01f, 0.02f,-0.04f, 0.06f,-0.02f, 0.05f,-0.02f};
  static const float cameraContrast[7]  = {1.02f, 1.15f, 1.08f, 1.22f, 0.85f, 1.18f, 1.05f};
  static const float cameraHighlight[7] = {0.12f, 0.06f, 0.25f, 0.05f, 0.30f, 0.04f, 0.10f};
  static const float cameraRGain[7]     = {1.04f, 1.06f, 1.18f, 0.88f, 1.12f, 0.94f, 1.03f};
  static const float cameraGGain[7]     = {1.00f, 0.98f, 0.94f, 1.04f, 0.98f, 1.08f, 0.98f};
  static const float cameraBGain[7]     = {0.96f, 1.02f, 0.78f, 1.16f, 0.82f, 1.04f, 1.02f};
  static const float cameraGamma[7]     = {1.02f, 0.94f, 1.08f, 0.90f, 1.15f, 0.96f, 1.00f};
  const int ci = clampi(s.camera - 1, 0, 6);
  static const int cameraParamIds[7][4] = {
    {P_FXC6_CHARACTER,P_FXC6_COLOR,P_FXC6_SIGNAL,P_FXC6_GRAIN},
    {P_GEN7_CHARACTER,P_GEN7_COLOR,P_GEN7_SIGNAL,P_GEN7_GRAIN},
    {P_JVC_C7_CHARACTER,P_JVC_C7_COLOR,P_JVC_C7_SIGNAL,P_JVC_C7_GRAIN},
    {P_R20_CHARACTER,P_R20_COLOR,P_R20_SIGNAL,P_R20_GRAIN},
    {P_FALL78_CHARACTER,P_FALL78_COLOR,P_FALL78_SIGNAL,P_FALL78_GRAIN},
    {P_ERC_CHARACTER,P_ERC_COLOR,P_ERC_SIGNAL,P_ERC_GRAIN},
    {P_JVC_SXM_CHARACTER,P_JVC_SXM_COLOR,P_JVC_SXM_SIGNAL,P_JVC_SXM_GRAIN}
  };
  const float cameraCharacter = getSlider(params,cameraParamIds[ci][0],100.0f);
  const float cameraColor = getSlider(params,cameraParamIds[ci][1],100.0f);
  const float cameraSignal = getSlider(params,cameraParamIds[ci][2],100.0f);
  const float cameraGrain = getSlider(params,cameraParamIds[ci][3],100.0f);
  const float cameraDrive = 1.0f-std::exp(-strength*0.70f);
  const float sat = s.saturation * (0.65f + cameraSat[ci]*0.35f + (cameraColor-0.5f)*0.45f*cameraDrive*s.continuous);
  const float soft = clamp01(s.softness * (0.25f+0.75f*clamp01(strength/4.0f)) * s.continuous + cameraSoft[ci]*(cameraCharacter*0.01f)*0.15f);
  const float bleed = clamp01(s.bleed * (0.55f + cameraBleed[ci]) * (0.2f+0.8f*clamp01(strength/4.0f)) * s.continuous);
  const bool grainEnabled = getCheck(params, P_GRAIN_ENABLE) || (s.grain > 0.001f);
  const bool trackingEnabled = getCheck(params, P_TRACKING_ENABLE) || (s.tracking > 0.001f);
  const bool tapeEnabled = getCheck(params, P_TAPE_ENABLE) || (s.tapeType > 0 && s.tapeStrength > 0.001f);
  const bool overlayEnabled = getCheck(params, P_OVERLAY_ENABLE);
  const bool otherEnabled = getCheck(params, P_OTHER_ENABLE) || (params[P_OTHER_ENGINE]->u.pd.value > 1 && getSlider(params, P_OTHER_STRENGTH, 100.0f) > 0.001f);
  const bool crtEnabled = getCheck(params, P_CRT_ENABLE) || (getSlider(params, P_CRT_SCANLINES, 100.0f) > 0.001f || getSlider(params, P_CRT_CURVE, 100.0f) > 0.001f || getSlider(params, P_CRT_RGB, 100.0f) > 0.001f);
  const bool transparentTapeEnabled = getCheck(params,P_TRANSPARENT_TAPE_ENABLE);
  const bool tapeSweepEnabled = getCheck(params,P_TAPE_SWEEP_ENABLE);
  const float transparentTapeStrength = getSlider(params,P_TRANSPARENT_TAPE_STRENGTH,100.0f)*0.01f;
  const float transparentTapeWidth = getSlider(params,P_TRANSPARENT_TAPE_WIDTH,100.0f)*0.01f;
  const float transparentTapeSpeed = getSlider(params,P_TRANSPARENT_TAPE_SPEED,100.0f);
  const float tapeSweepStrength = getSlider(params,P_TAPE_SWEEP_STRENGTH,100.0f)*0.01f;
  const float tapeSweepWidth = getSlider(params,P_TAPE_SWEEP_WIDTH,100.0f)*0.01f;
  const float tapeSweepSpeed = getSlider(params,P_TAPE_SWEEP_SPEED,100.0f);
  const float tapeSweepInterval = std::max(0.1f,getSlider(params,P_TAPE_SWEEP_INTERVAL,1.0f));
  const float crtCurveAmount = crtEnabled ? getSlider(params,P_CRT_CURVE,100.0f)*0.01f : 0.0f;
  const float crtRgbAmount = crtEnabled ? getSlider(params,P_CRT_RGB,100.0f)*0.01f : 0.0f;
  const float grainAmount = grainEnabled ? (s.grain + cameraNoise[ci]*cameraGrain) : 0.0f;
  const float fps = (in_data->time_step != 0) ? (float)in_data->time_scale / (float)in_data->time_step : 24.0f;
  const float eventPeriod = s.eventRate > 0.001f ? 60.0f / s.eventRate : 1.0e9f;
  const int eventSlot = (int)std::floor(timeSec / eventPeriod);
  const float slotTime = timeSec - eventSlot * eventPeriod;
  const bool chosenEvent = s.events && s.eventRate > 0.0f && rand01((uint32_t)(eventSlot * 747796405u) ^ (uint32_t)s.seed) < 0.68f;
  float eventEnv = 0.0f;
  if (chosenEvent) {
    const float attack = 0.015f;
    if (slotTime < attack) eventEnv = smooth(0.0f, attack, slotTime);
    else if (slotTime < s.eventDuration) eventEnv = 1.0f;
    else if (s.recovery > 0.0f) eventEnv = 1.0f - smooth(s.eventDuration, s.eventDuration + s.recovery, slotTime);
    else eventEnv = 0.0f;
  }

  PF_ParamDef overlayCheckout;
  AEFX_CLR_STRUCT(overlayCheckout);
  PF_EffectWorld *overlayWorld = nullptr;
  const float overlaySpeed = getSlider(params,P_OVERLAY_SPEED,100.0f);
  const float overlayOffset = getSlider(params,P_OVERLAY_OFFSET,1.0f);
  const bool overlayLoop = getCheck(params,P_OVERLAY_LOOP);
  const float loopPeriod = std::max(0.1f,getSlider(params,P_OVERLAY_LOOP_PERIOD,1.0f));
  float overlayTimeSec = timeSec*overlaySpeed+overlayOffset;
  if (overlayLoop && loopPeriod > 0.0f) { overlayTimeSec = std::fmod(overlayTimeSec,loopPeriod); if (overlayTimeSec < 0.0f) overlayTimeSec += loopPeriod; }
  const A_long overlayTime = (A_long)std::round(overlayTimeSec * (float)in_data->time_scale);
  const PF_Err overlayCheckoutErr = PF_CHECKOUT_PARAM(in_data, P_OVERLAY_LAYER, overlayTime, in_data->time_step, in_data->time_scale, &overlayCheckout);
  const bool overlayCheckedOut = (overlayCheckoutErr == PF_Err_NONE);
  if (overlayCheckedOut && overlayCheckout.u.ld.data) overlayWorld = &overlayCheckout.u.ld;
  const float overlayOpacity = getSlider(params,P_OVERLAY_OPACITY,100.0f);
  const int overlayMode = params[P_OVERLAY_MODE]->u.pd.value - 1;
  const int overlayStride = overlayWorld ? overlayWorld->rowbytes/(int)sizeof(PF_Pixel) : 0;
  const PF_Pixel *overlayBase = overlayWorld ? (const PF_Pixel*)overlayWorld->data : nullptr;
  const int otherEngine = params[P_OTHER_ENGINE]->u.pd.value - 1;
  const float otherStrength = otherEnabled ? getSlider(params,P_OTHER_STRENGTH,100.0f)*clamp01(strength/5.0f) : 0.0f;
  const float craftTracking=getSlider(params,P_CRAFT_TRACKING,100.0f);
  const float craftChroma=getSlider(params,P_CRAFT_CHROMA,100.0f);
  const float craftHead=getSlider(params,P_CRAFT_HEADSWITCH,100.0f);
  const float craftSmear=getSlider(params,P_CRAFT_SMEAR,100.0f);
  const float legendBreak=getSlider(params,P_LEGEND_BREAKUP,100.0f);
  const float legendTear=getSlider(params,P_LEGEND_TEAR,100.0f);
  const float legendLoss=getSlider(params,P_LEGEND_SIGNAL,100.0f);
  const float legendBlocks=getSlider(params,P_LEGEND_BLOCKS,100.0f);
  const float ntscChroma=getSlider(params,P_NTSC_CHROMA_DELAY,100.0f);
  const float ntscHead=getSlider(params,P_NTSC_HEAD_SWITCH,100.0f);
  const float ntscWave=getSlider(params,P_NTSC_EDGE_WAVE,100.0f);
  const float ntscComposite=getSlider(params,P_NTSC_COMPOSITE,100.0f);
  const float signalStrength=getSlider(params,P_SIGNAL_STRENGTH,100.0f);
  const float signalLuma=getSlider(params,P_SIGNAL_LUMA_NOISE,100.0f);
  const float signalChroma=getSlider(params,P_SIGNAL_CHROMA_NOISE,100.0f);
  const float signalErrors=getSlider(params,P_SIGNAL_TAPE_ERRORS,100.0f);

  for (int y=0; y<h; ++y) {
    PF_Pixel *dst = outBase + y*outStride;
    for (int x=0; x<w; ++x) {
      int sx = clampi((int)((x + 0.5f) * inW / std::max(1,w)),0,inW-1);
      int sy = clampi((int)((y + 0.5f) * inH / std::max(1,h)),0,inH-1);
      // CRT barrel geometry is a real source-coordinate warp, not only an edge vignette.
      if (crtCurveAmount > 0.001f) {
        const float nx=((float)x/std::max(1,w-1)-0.5f)*2.0f;
        const float ny=((float)y/std::max(1,h-1)-0.5f)*2.0f;
        const float rr=nx*nx+ny*ny;
        const float k=1.0f+crtCurveAmount*0.12f*rr;
        sx=clampi((int)std::round(((nx*k*0.5f+0.5f)*inW)),0,inW-1);
        sy=clampi((int)std::round(((ny*k*0.5f+0.5f)*inH)),0,inH-1);
      }
      PF_Pixel p = inBase[sy*inStride + sx];
      float r=p.red/255.0f, g=p.green/255.0f, b=p.blue/255.0f;
      if (s.continuous && strength > 0.0001f) {
        // Mild luminance softness, implemented as a real spatial filter.
        const int blurRadius = clampi((int)std::ceil(soft*3.0f),0,3);
        if (blurRadius>0) {
          float rr=0,gg=0,bb=0,weight=0;
          for(int k=-blurRadius;k<=blurRadius;++k){
            const int xx=clampi(sx+k,0,inW-1); const float kw=(float)(blurRadius+1-std::abs(k));
            const PF_Pixel q=inBase[sy*inStride+xx]; rr+=q.red*kw;gg+=q.green*kw;bb+=q.blue*kw;weight+=kw;
          }
          r=(r*(1.0f-soft)+rr/weight/255.0f*soft); g=(g*(1.0f-soft)+gg/weight/255.0f*soft); b=(b*(1.0f-soft)+bb/weight/255.0f*soft);
        }
        // Chroma separation: sample color from a horizontally shifted source pixel.
        const int cx=clampi(sx+(int)std::round(s.chromaX*strength),0,inW-1);
        const int cy=clampi(sy+(int)std::round(s.chromaY*strength),0,inH-1);
        const PF_Pixel cp=inBase[cy*inStride+cx];
        const float cbR=cp.red/255.0f, cbG=cp.green/255.0f, cbB=cp.blue/255.0f;
        r=r*(1.0f-clamp01(bleed))+cbR*clamp01(bleed);
        g=g*(1.0f-clamp01(bleed))+cbG*clamp01(bleed);
        b=b*(1.0f-clamp01(bleed))+cbB*clamp01(bleed);
        const float lum=0.2126f*r+0.7152f*g+0.0722f*b;
        r=lum+(r-lum)*sat; g=lum+(g-lum)*sat; b=lum+(b-lum)*sat;
        // Camera-specific color crossover, tonal curve and highlight shoulder. Strength
        // is normalized only for this calibrated stage so 2000 remains controllable.
        const float camMix=clamp01((cameraCharacter*0.01f) * cameraDrive);
        const float warm=cameraWarm[ci]*camMix;
        const float greenBias=cameraGreen[ci]*camMix;
        r += warm*(0.35f+lum) - greenBias*0.25f;
        g += greenBias*(0.25f+lum) - warm*0.08f;
        b -= warm*(0.30f+lum) + greenBias*0.12f;
        const float camContrast=1.0f+(cameraContrast[ci]-1.0f)*camMix;
        r=(r-0.5f)*camContrast+0.5f; g=(g-0.5f)*camContrast+0.5f; b=(b-0.5f)*camContrast+0.5f;
        const float shoulder=clamp01(cameraHighlight[ci]*camMix);
        r -= shoulder*std::max(0.0f,r-0.68f)*std::max(0.0f,r-0.68f);
        g -= shoulder*std::max(0.0f,g-0.68f)*std::max(0.0f,g-0.68f);
        b -= shoulder*std::max(0.0f,b-0.68f)*std::max(0.0f,b-0.68f);
        r=clamp01(r*(1.0f+(cameraRGain[ci]-1.0f)*camMix));
        g=clamp01(g*(1.0f+(cameraGGain[ci]-1.0f)*camMix));
        b=clamp01(b*(1.0f+(cameraBGain[ci]-1.0f)*camMix));
        const float gamma=1.0f+(cameraGamma[ci]-1.0f)*camMix;
        r=std::pow(clamp01(r),gamma); g=std::pow(clamp01(g),gamma); b=std::pow(clamp01(b),gamma);
        const float black=s.black*0.35f*std::min(2.0f,strength);
        r=clamp01((r-0.5f)* (1.0f+black*0.01f)+0.5f-black*0.005f);
        g=clamp01((g-0.5f)* (1.0f+black*0.01f)+0.5f-black*0.005f);
        b=clamp01((b-0.5f)* (1.0f+black*0.01f)+0.5f-black*0.005f);
        const float scale=std::max(1.0f,s.grainScale);
        const uint32_t motionFrame=(uint32_t)std::floor(timeSec*s.grainSpeed*fps);
        const uint32_t nseed=(uint32_t)s.seed ^ (motionFrame*2246822519u) ^ (uint32_t)((x/(int)scale)*73856093u) ^ (uint32_t)((y/(int)scale)*19349663u);
        const float n=(rand01(nseed)-0.5f)*std::min(0.28f, grainAmount*0.12f)*(0.25f+0.75f*clamp01(strength/4.0f));
        r=clamp01(r+n*1.15f); g=clamp01(g+n*0.80f); b=clamp01(b+n*1.30f);
      }
      // Tracking bands are time-deterministic and vary by camera profile.
      if (s.continuous && trackingEnabled && s.tracking>0.001f) {
        const float phase=timeSec*(0.08f+0.8f*s.trackFreq)*(1.0f+ci*0.07f);
        const float bandY=std::fmod(phase,1.0f);
        const float yn=(float)y/std::max(1,h);
        const float dist=std::fabs(yn-bandY);
        const float bw=std::max(0.002f,s.trackWidth*0.08f);
        const float band=(1.0f-smooth(bw,bw*2.5f,dist))*s.tracking*cameraSignal*strength;
        r=clamp01(r+band*0.30f); g=clamp01(g+band*0.42f); b=clamp01(b+band*0.65f);
      }
      // Discrete glitch event: horizontal tearing, temporary channel shift, and dropout.
      if (eventEnv>0.001f) {
        const uint32_t eh=(uint32_t)s.seed ^ (uint32_t)(eventSlot*2654435761u) ^ (uint32_t)(y*2246822519u);
        const float lineNoise=rand01(eh);
        if (lineNoise < 0.12f*eventEnv) { r=clamp01(r+0.35f*eventEnv); g*=1.0f-0.5f*eventEnv; b=clamp01(b+0.45f*eventEnv); }
        if (lineNoise < s.dropout*cameraSignal*eventEnv*0.5f) { r*=0.08f; g*=0.08f; b*=0.08f; }
        if (lineNoise < s.blocks*cameraSignal*eventEnv*0.35f) { const int block=((x/16)+(y/8)*13+eventSlot)%5; if(block<2){r=rand01(eh+7);g=rand01(eh+11);b=rand01(eh+19);} }
      }
      // Tape band family: independent of general event glitches.
      if (tapeEnabled && s.tapeType>0 && s.tapeStrength>0.001f) {
        const float speed=s.tapeSpeed*0.01f;
        float pos=std::fmod(timeSec*speed*0.12f+0.17f,1.0f);
        if (s.tapeDir==1 || s.tapeDir==3) pos=1.0f-pos;
        const float yn=(float)y/std::max(1,h), xn=(float)x/std::max(1,w);
        float dist=1.0f;
        if(s.tapeType==0) dist=1.0f;
        else if(s.tapeType==1) dist=(s.tapeDir>=2) ? std::fabs(xn-pos) : std::fabs(yn-pos);
        else if(s.tapeType==2) dist=std::fabs(yn-(s.tapeDir==0 ? s.tapeSize*0.01f*0.25f : 1.0f-s.tapeSize*0.01f*0.25f));
        else if(s.tapeType==3 || s.tapeType==4) dist=(s.tapeDir>=2) ? std::fabs(yn-pos) : std::fabs(xn-pos);
        else dist=rand01((uint32_t)(x*31+y*71+frame*997+s.seed));
        const float width=s.tapeSize/100.0f*0.10f+0.003f;
        const float mask=(s.tapeType==5 ? clamp01(1.0f-dist) : 1.0f-smooth(width,width*2.0f,dist));
        const float ts=clamp01(mask*(s.tapeStrength/100.0f)*(0.2f+0.8f*clamp01(strength/5.0f)));
        if (ts > 0.001f && (s.tapeType==1 || s.tapeType==2 || s.tapeType==3 || s.tapeType==4)) {
          // Disturb the source image inside the tape band. This is the key difference
          // between a signal/tape event and simply painting a colored stripe over footage.
          int tx=sx, ty=sy;
          const float irregular=std::sin((float)y*0.071f+timeSec*8.3f)*0.55f+
                                std::sin((float)y*0.019f-timeSec*3.7f)*0.45f;
          const int displacement=(int)std::round(irregular*ts*std::max(2,w/24));
          if (s.tapeType==1 || s.tapeType==2) tx=clampi(sx+displacement,0,inW-1);
          else if (s.tapeType==3) ty=clampi(sy+displacement,0,inH-1);
          else tx=clampi(sx+(displacement*(rand01((uint32_t)(y*73+s.seed+frame*19))>0.5f?2: -2)),0,inW-1);
          const PF_Pixel tp=inBase[ty*inStride+tx];
          const float tr=tp.red/255.0f, tg=tp.green/255.0f, tb=tp.blue/255.0f;
          r=r*(1.0f-ts)+tr*ts; g=g*(1.0f-ts)+tg*ts; b=b*(1.0f-ts)+tb*ts;
          // Tape head switching and tearing have irregular luma loss plus chroma phase error.
          const float phase=std::sin((float)y*0.31f+timeSec*19.0f)*ts*0.10f;
          if (s.tapeType==2) { r*=1.0f-ts*0.25f; g*=1.0f-ts*0.35f; b*=1.0f-ts*0.18f; }
          if (s.tapeType==4) { r=clamp01(r+phase+ts*0.12f); g=clamp01(g-phase); b=clamp01(b-phase-ts*0.08f); }
        } else if (s.tapeType==5 && ts>0.001f) {
          const float snow=(rand01((uint32_t)(x*31+y*71+frame*997+s.seed))-0.5f)*ts*0.8f;
          r=clamp01(r+snow); g=clamp01(g+snow); b=clamp01(b+snow);
        }
      }
      if (otherEngine>0 && otherStrength>0.0001f) {
        const float xn=(float)x/std::max(1,w), yn=(float)y/std::max(1,h);
        const uint32_t oh=(uint32_t)s.seed ^ (uint32_t)(frame*2246822519u) ^ (uint32_t)(x*3266489917u) ^ (uint32_t)(y*668265263u);
        if (otherEngine==1) { // VHS Craft-inspired: moving tracking interference, chroma smear, head switching.
          const float trackPos=std::fmod(timeSec*0.23f+0.31f,1.0f);
          const float track=1.0f-smooth(0.008f,0.025f,std::fabs(yn-trackPos));
          const float head=1.0f-smooth(0.01f,0.06f,std::fabs(yn-(1.0f-0.025f)));
          const float n=rand01(oh)-0.5f;
          const float t=otherStrength*(craftTracking*track+craftHead*head)*0.006f;
          r=clamp01(r+t+n*craftSmear*otherStrength*0.15f);
          g=clamp01(g-t*0.25f+n*craftSmear*otherStrength*0.15f);
          b=clamp01(b+t*0.7f+craftChroma*otherStrength*0.12f*n);
        } else if (otherEngine==2) { // Independent Crates/VHS-Legend reference-style breakup, not their shader/runtime.
          const float line=(rand01((uint32_t)(y/3+frame*41+s.seed))-0.5f)*2.0f;
          const bool tear=rand01((uint32_t)(y/7+x/29+frame*17+s.seed)) < legendTear*otherStrength*0.08f;
          const bool loss=rand01((uint32_t)(x/12+y/8+frame*73+s.seed)) < legendLoss*otherStrength*0.08f;
          r=clamp01(r+line*legendBreak*otherStrength*0.35f);
          g=clamp01(g+line*legendBreak*otherStrength*0.18f);
          b=clamp01(b-line*legendBreak*otherStrength*0.28f);
          if(tear){r=clamp01(r+0.18f*otherStrength);g*=0.55f;b=clamp01(b+0.25f*otherStrength);}
          if(loss){r*=0.15f;g*=0.15f;b*=0.15f;}
          if(rand01(oh)<legendBlocks*otherStrength*0.06f){r=rand01(oh+11);g=rand01(oh+23);b=rand01(oh+37);}
        } else if (otherEngine==3) { // NTSC composite-like edge wave, chroma delay, and head switching.
          const float wave=std::sin(yn*90.0f+timeSec*7.0f)*ntscWave*otherStrength*0.12f;
          const float phase=std::sin((xn*120.0f+timeSec*18.0f))*ntscComposite*otherStrength*0.08f;
          const float delay=(rand01((uint32_t)(y*31+frame*13+s.seed))-0.5f)*ntscChroma*otherStrength*0.08f;
          const float head=(yn>0.965f)?ntscHead*otherStrength*0.2f:0.0f;
          r=clamp01(r+wave+phase+delay);
          g=clamp01(g+wave-phase);
          b=clamp01(b-wave-delay+head);
        } else if (otherEngine==4) { // Signal-style luma/chroma noise and tape errors.
          const float l=(rand01(oh+3)-0.5f)*signalLuma*signalStrength*otherStrength*0.12f;
          const float c=(rand01(oh+7)-0.5f)*signalChroma*signalStrength*otherStrength*0.18f;
          const bool error=rand01((uint32_t)(x/8+y/4+frame*29+s.seed))<signalErrors*signalStrength*otherStrength*0.08f;
          r=clamp01(r+l+c); g=clamp01(g+l-c*0.25f); b=clamp01(b+l-c);
          if(error){r*=0.2f;g*=0.2f;b*=0.2f;}
        }
      }
      // Procedural transparent tape: irregular semi-transparent signal abrasion,
      // not a stack of uniform white strokes. Its shape and chroma response are time-driven.
      if (transparentTapeEnabled && transparentTapeStrength>0.001f) {
        const float yn=(float)y/(float)std::max(1,h-1);
        const float travel=std::fmod(timeSec*(0.08f+transparentTapeSpeed*0.006f),1.0f);
        const float wobble=0.012f*std::sin((float)x*0.021f+timeSec*4.7f)+0.006f*std::sin((float)x*0.067f-timeSec*2.9f);
        const float center=travel + wobble;
        const float halfWidth=0.002f+transparentTapeWidth*0.045f;
        const float edge=1.0f-smooth(halfWidth,halfWidth*1.9f,std::fabs(yn-center));
        const float broken=0.55f+0.45f*rand01((uint32_t)(x/11+y/3+(int)(timeSec*17.0f)+s.seed));
        const float a=clamp01(edge*broken*transparentTapeStrength*0.72f);
        if (a>0.001f) {
          const float l=0.2126f*r+0.7152f*g+0.0722f*b;
          const float chromaPhase=std::sin((float)x*0.047f+timeSec*12.0f)*a*0.16f;
          // Blend toward locally altered signal luminance/chroma; avoid flat white-only marks.
          r=clamp01(r*(1.0f-a*0.28f)+l*a*0.28f+chromaPhase);
          g=clamp01(g*(1.0f-a*0.20f)+l*a*0.20f-chromaPhase*0.35f);
          b=clamp01(b*(1.0f-a*0.32f)+l*a*0.32f+chromaPhase*0.65f);
        }
      }
      // Tape sweep is a discrete moving tracking failure: displace the source and shift
      // chroma while the band crosses the frame, then recover outside the event.
      if (tapeSweepEnabled && tapeSweepStrength>0.001f) {
        const float slot=std::floor(timeSec/tapeSweepInterval);
        const float slotTime=timeSec-slot*tapeSweepInterval;
        const float sweepDuration=std::max(0.15f,tapeSweepInterval*100.0f/std::max(1.0f,tapeSweepSpeed)*0.25f);
        const float phase=clamp01(slotTime/sweepDuration);
        const float center=phase;
        const float yn=(float)y/(float)std::max(1,h-1);
        const float wobble=0.006f*std::sin((float)x*0.031f+timeSec*7.0f)+0.004f*std::sin((float)x*0.079f-timeSec*5.0f);
        const float halfWidth=0.004f+tapeSweepWidth*0.075f;
        const float band=1.0f-smooth(halfWidth,halfWidth*1.8f,std::fabs(yn-center-wobble));
        const float envelope=clamp01(band*tapeSweepStrength*(slotTime<sweepDuration ? 1.0f : 0.0f));
        if (envelope>0.001f) {
          const int dx=(int)std::round(std::sin((float)y*0.11f+timeSec*9.0f)*envelope*std::max(2,w/18));
          const PF_Pixel moved=inBase[sy*inStride+clampi(sx+dx,0,inW-1)];
          const PF_Pixel redShift=inBase[sy*inStride+clampi(sx+dx+2,0,inW-1)];
          const PF_Pixel blueShift=inBase[sy*inStride+clampi(sx+dx-2,0,inW-1)];
          const float mr=moved.red/255.0f, mg=moved.green/255.0f, mb=moved.blue/255.0f;
          r=clamp01(r*(1.0f-envelope)+mr*envelope+envelope*(redShift.red/255.0f-mr)*0.35f);
          g=clamp01(g*(1.0f-envelope)+mg*envelope);
          b=clamp01(b*(1.0f-envelope)+mb*envelope+envelope*(blueShift.blue/255.0f-mb)*0.35f);
          const float loss=envelope*(0.08f+0.12f*rand01((uint32_t)(y/2+frame*17+s.seed)));
          r*=1.0f-loss; g*=1.0f-loss; b*=1.0f-loss;
        }
      }
      // CRT display stage: image-dependent scanline modulation, channel convergence,
      // edge falloff and time-based flicker. It is strictly opt-in.
      if (crtEnabled) {
        const float xn=(float)x/(float)std::max(1,w-1), yn=(float)y/(float)std::max(1,h-1);
        const float scan=getSlider(params,P_CRT_SCANLINES,100.0f)*0.01f;
        const float curve=getSlider(params,P_CRT_CURVE,100.0f)*0.01f;
        const float glow=getSlider(params,P_CRT_GLOW,100.0f)*0.01f;
        const float rgb=getSlider(params,P_CRT_RGB,100.0f)*0.01f;
        const float flicker=getSlider(params,P_CRT_FLICKER,100.0f)*0.01f;
        const float vignette=getSlider(params,P_CRT_VIGNETTE,100.0f)*0.01f;
        const float scanWave=0.5f+0.5f*std::cos((float)y*3.14159265f);
        const float scanGain=1.0f-scan*0.30f*scanWave;
        const float dx=xn-0.5f, dy=yn-0.5f;
        const float edge=clamp01(1.0f-vignette*1.35f*(dx*dx*1.8f+dy*dy*1.8f));
        const float flick=1.0f-flicker*0.035f*(0.5f+0.5f*std::sin(timeSec*37.0f));
        const float curved=1.0f-curve*0.16f*(dx*dx+dy*dy);
        const float l=0.2126f*r+0.7152f*g+0.0722f*b;
        const float halo=std::max(0.0f,l-0.68f)*glow*0.16f;
        const float convergence=rgb*0.035f;
        float cr=r, cb=b;
        if (crtRgbAmount>0.001f) {
          const int off=clampi((int)std::round(crtRgbAmount*3.0f),0,3);
          const PF_Pixel rp=inBase[sy*inStride+clampi(sx+off,0,inW-1)];
          const PF_Pixel bp=inBase[sy*inStride+clampi(sx-off,0,inW-1)];
          cr=rp.red/255.0f; cb=bp.blue/255.0f;
        }
        r=clamp01((r*(1.0f-convergence)+cr*convergence+convergence*(g-b))*scanGain*edge*flick*curved+halo);
        g=clamp01(g*scanGain*edge*flick*curved+halo*0.9f);
        b=clamp01((b*(1.0f-convergence)+cb*convergence+convergence*(g-r))*scanGain*edge*flick*curved+halo*1.05f);
      }
      if (overlayEnabled && overlayBase && overlayOpacity > 0.0f && overlayWorld->width > 0 && overlayWorld->height > 0) {
        const int ox=clampi((int)((x+0.5f)*overlayWorld->width/std::max(1,w)),0,overlayWorld->width-1);
        const int oy=clampi((int)((y+0.5f)*overlayWorld->height/std::max(1,h)),0,overlayWorld->height-1);
        const PF_Pixel op=overlayBase[oy*overlayStride+ox];
        float orr=op.red/255.0f, og=op.green/255.0f, ob=op.blue/255.0f;
        float a=(overlayOpacity/100.0f)*(op.alpha/255.0f);
        if (overlayMode==1) { // Integrated Blend: treat the overlay as a centered signal disturbance, not an opaque layer.
          r=clamp01(r+(orr-0.5f)*a*0.85f); g=clamp01(g+(og-0.5f)*a*0.85f); b=clamp01(b+(ob-0.5f)*a*0.85f);
        } else if (overlayMode==2) { r=clamp01(r+orr*a); g=clamp01(g+og*a); b=clamp01(b+ob*a); }
        else if (overlayMode==3) { r=1.0f-(1.0f-r)*(1.0f-orr*a); g=1.0f-(1.0f-g)*(1.0f-og*a); b=1.0f-(1.0f-b)*(1.0f-ob*a); }
        else if (overlayMode==4) { r=r*(1.0f-a)+r*orr*a; g=g*(1.0f-a)+g*og*a; b=b*(1.0f-a)+b*ob*a; }
        else if (overlayMode==5) { const float key=0.2126f*orr+0.7152f*og+0.0722f*ob; a*=smooth(0.08f,0.45f,key); r=r*(1.0f-a)+orr*a; g=g*(1.0f-a)+og*a; b=b*(1.0f-a)+ob*a; }
        else { r=r*(1.0f-a)+orr*a; g=g*(1.0f-a)+og*a; b=b*(1.0f-a)+ob*a; }
      }
      dst[x].red=(uint8_t)std::round(clamp01(r)*255.0f);
      dst[x].green=(uint8_t)std::round(clamp01(g)*255.0f);
      dst[x].blue=(uint8_t)std::round(clamp01(b)*255.0f);
      dst[x].alpha=p.alpha;
    }
  }
  if (overlayCheckedOut) PF_CHECKIN_PARAM(in_data, &overlayCheckout);
  return err;
}

static PF_Err AudioRender(PF_InData *in_data, PF_OutData *out_data, PF_ParamDef *params[]) {
  PF_SoundWorld *src = &in_data->src_snd;
  PF_SoundWorld *dst = &out_data->dest_snd;
  if (!src->dataP || !dst->dataP || src->num_samples <= 0 || dst->num_samples <= 0) return PF_Err_NONE;
  const A_long srcChannels = std::max<A_long>(1, std::min<A_long>(2, src->fi.num_channels));
  const A_long dstChannels = std::max<A_long>(1, std::min<A_long>(2, dst->fi.num_channels));
  const A_long frames = std::min(src->num_samples, dst->num_samples);
  const float mix = clamp01((float)params[P_AUDIO_MIX]->u.fs_d.value / 100.0f);
  const float eventAmount = clamp01((float)params[P_AUDIO_EVENT_AMOUNT]->u.fs_d.value / 100.0f);
  const float crackle = clamp01((float)params[P_AUDIO_CRACKLE]->u.fs_d.value / 100.0f);
  const float dropout = clamp01((float)params[P_AUDIO_DROPOUT]->u.fs_d.value / 100.0f);
  const float saturation = clamp01((float)params[P_AUDIO_SATURATION]->u.fs_d.value / 100.0f);
  const float lofi = clamp01((float)params[P_AUDIO_LOFI]->u.fs_d.value / 100.0f);
  const int seed = params[P_SEED]->u.sd.value;
  const float rate = (float)std::max(1.0, (double)dst->fi.rateF);
  const bool enabled = params[P_AUDIO_ENABLE]->u.bd.value != 0;
  const float eventRate = (float)params[P_EVENT_RATE]->u.fs_d.value;
  const float eventDuration = (float)params[P_EVENT_DURATION]->u.fs_d.value;
  const float recovery = (float)params[P_RECOVERY]->u.fs_d.value;
  const float eventPeriod = eventRate > 0.001f ? 60.0f / eventRate : 1.0e9f;
  const float *inSamples = (const float*)src->dataP;
  float *outSamples = (float*)dst->dataP;
  for (A_long i=0; i<frames; ++i) {
    const double t = ((double)in_data->start_sampL + (double)i) / rate;
    const int slot = (int)std::floor(t / eventPeriod);
    const float slotTime = (float)(t - (double)slot * eventPeriod);
    const bool eventSelected = params[P_AUDIO_EVENTS]->u.bd.value != 0 && eventRate > 0.0f && rand01((uint32_t)(slot * 747796405u) ^ (uint32_t)seed) < 0.68f;
    float env = 0.0f;
    if (eventSelected) {
      const float attack=0.015f;
      if (slotTime<attack) env=smooth(0.0f,attack,slotTime);
      else if (slotTime<eventDuration) env=1.0f;
      else if (recovery>0.0f) env=1.0f-smooth(eventDuration,eventDuration+recovery,slotTime);
    }
    const uint32_t sampleKey=(uint32_t)(in_data->start_sampL+i)*2246822519u ^ (uint32_t)seed;
    const float noise=(rand01(sampleKey)-0.5f)*2.0f*crackle*eventAmount*env*0.35f;
    const uint32_t dropoutKey=(uint32_t)((in_data->start_sampL+i)/256)*3266489917u ^ (uint32_t)(slot*668265263u) ^ (uint32_t)seed;
    const bool drop = eventSelected && env>0.0f && rand01(dropoutKey) < dropout*eventAmount*env*0.45f;
    for (A_long ch=0; ch<dstChannels; ++ch) {
      const A_long srcIndex=i*srcChannels+std::min(ch,srcChannels-1);
      const A_long dstIndex=i*dstChannels+ch;
      const float dry=inSamples[srcIndex];
      float wet=drop ? 0.0f : dry;
      wet += noise;
      // Soft saturation is a real sample-domain nonlinear stage.
      if (saturation>0.0f) {
        const float drive=1.0f+saturation*8.0f;
        wet=std::tanh(wet*drive)/std::tanh(drive);
      }
      // Deterministic bit-depth reduction; does not rely on previous playback state.
      if (lofi>0.0f) {
        const int bits=clampi((int)std::round(16.0f-lofi*12.0f),4,16);
        const float levels=(float)((1u<<bits)-1u);
        wet=std::round(clamp01(wet*0.5f+0.5f)*levels)/levels*2.0f-1.0f;
      }
      outSamples[dstIndex]=enabled ? dry*(1.0f-mix)+wet*mix : dry;
    }
  }
  // If destination has more samples than source, clear the tail rather than read out of bounds.
  for (A_long i=frames*dstChannels; i<dst->num_samples*dstChannels; ++i) outSamples[i]=0.0f;
  return PF_Err_NONE;
}

extern "C" DllExport PF_Err EffectMain(PF_Cmd cmd, PF_InData *in_data, PF_OutData *out_data, PF_ParamDef *params[], PF_LayerDef *output, void *) {
  PF_Err err = PF_Err_NONE;
  switch (cmd) {
    case PF_Cmd_ABOUT: err = About(in_data, out_data, params, output); break;
    case PF_Cmd_GLOBAL_SETUP: err = GlobalSetup(out_data); break;
    case PF_Cmd_PARAMS_SETUP: err = ParamsSetup(in_data, out_data); break;
    case PF_Cmd_RENDER: err = Render(in_data, params, output); break;
    case PF_Cmd_AUDIO_RENDER: err = AudioRender(in_data, out_data, params); break;
    default: break;
  }
  return err;
}
