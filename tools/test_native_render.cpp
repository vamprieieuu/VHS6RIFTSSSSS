#ifndef _STANDALONE_TEST_HARNESS_
#define _STANDALONE_TEST_HARNESS_
#endif

#include <stdint.h>
#include <stddef.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <functional>
#include <algorithm>
#include <cstdio>

#ifndef DllExport
#define DllExport
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

typedef unsigned char A_u_char;
typedef unsigned short A_u_short;
typedef unsigned int A_u_long;
typedef int A_long;
typedef short A_short;
typedef double PF_FpLong;
typedef float PF_FpShort;
typedef int PF_Err;
typedef int PF_Fixed;
typedef void* A_Handle;
typedef void* PF_Handle;
typedef void* PF_ArbitraryH;
typedef unsigned char PF_Boolean;
typedef int PF_TransferMode;
typedef int PF_ValueDisplayFlags;
typedef int PF_FSliderFlags;
typedef int PF_ParamValue;

#define PF_Err_NONE 0
#define PF_Err_OUT_OF_MEMORY 1

typedef struct {
	A_u_char alpha, red, green, blue;
} PF_Pixel8;

typedef PF_Pixel8 PF_Pixel;

typedef struct {
	int width;
	int height;
	int rowbytes;
	void *data;
} PF_LayerDef;

typedef PF_LayerDef PF_EffectWorld;

typedef struct {
	int time_scale;
	int current_time;
	int time_step;
	PF_LayerDef src_snd;
} PF_InData;

typedef struct {
	int my_version;
	int out_flags;
	int out_flags2;
	int num_params;
	char return_msg[512];
	PF_LayerDef dest_snd;
} PF_OutData;

typedef struct {
	union {
		struct { int value; } bd;
		struct { double value; } fs_d;
		struct { int value; } pd;
		struct { int value; } sd;
		PF_LayerDef ld;
	} u;
} PF_ParamDef;

typedef int PF_Cmd;
#define PF_Cmd_ABOUT 1
#define PF_Cmd_GLOBAL_SETUP 2
#define PF_Cmd_PARAMS_SETUP 3
#define PF_Cmd_RENDER 4
#define PF_Cmd_AUDIO_SETUP 5
#define PF_Cmd_AUDIO_SETDOWN 6
#define PF_Cmd_AUDIO_RENDER 7

typedef struct {
	void *dataP;
	int num_samples;
	struct { float rateF; int num_channels; } fi;
} PF_SoundWorld;

#define PF_CHECKOUT_PARAM(in_data, index, time, time_step, time_scale, param_def) PF_Err_NONE
#define PF_CHECKIN_PARAM(in_data, param_def) PF_Err_NONE

#define PF_SPRINTF(...) (0)
#define AEFX_CLR_STRUCT(s) memset(&(s), 0, sizeof(s))
#define PF_ADD_PARAM(in_data, id, def) PF_Err_NONE
#define PF_ADD_TOPIC(NAME, ID) PF_Err_NONE
#define PF_END_TOPIC(ID) PF_Err_NONE
#define PF_ADD_SLIDER(NAME, V_MIN, V_MAX, S_MIN, S_MAX, DFLT, ID) PF_Err_NONE
#define PF_ADD_FLOAT_SLIDER(NAME, V_MIN, V_MAX, S_MIN, S_MAX, DFLT, PREC, ID) PF_Err_NONE
#define PF_ADD_FLOAT_SLIDERX(NAME, V_MIN, V_MAX, S_MIN, S_MAX, DFLT, PREC, DISP, FLAGS, ID) PF_Err_NONE
#define PF_ADD_CHECKBOX(NAME, DESC, DFLT, FLAGS, ID) PF_Err_NONE
#define PF_ADD_POPUP(NAME, NUM, DFLT, CHOICES, ID) PF_Err_NONE
#define PF_ADD_LAYER(NAME, DFLT, ID) PF_Err_NONE
#define PF_LayerDefault_NONE 0
#define PF_Precision_TENTHS 1
#define PF_Precision_HUNDREDTHS 2
#define PF_VERSION(maj, min, bug, stage, build) 557057
#define PF_Stage_DEVELOP 0
#define PF_Stage_RELEASE 3
#define PF_OutFlag_DEEP_COLOR_AWARE 0x02000000
#define PF_OutFlag_PIX_INDEPENDENT 0x00000400
#define PF_OutFlag_WIDE_TIME_INPUT 0x00000002
#define PF_OutFlag_NON_PARAM_VARY 0x00000004
#define PF_OutFlag_AUDIO_EFFECT_TOO 0x40000000
#define PF_OutFlag_AUDIO_FLOAT_ONLY 0x08000000
#define PF_OutFlag2_NONE 0

#include "VHS6RIFT.h"
#include "../src/VHS6RIFT.cpp"

static void SavePPM(const char *filename, const PF_Pixel *buffer, int w, int h) {
	FILE *f = fopen(filename, "wb");
	if (!f) return;
	fprintf(f, "P6\n%d %d\n255\n", w, h);
	for (int i = 0; i < w * h; ++i) {
		fputc(buffer[i].red, f);
		fputc(buffer[i].green, f);
		fputc(buffer[i].blue, f);
	}
	fclose(f);
}

int main() {
	std::cout << "======================================================================\n";
	std::cout << "VHS Master Native C++ Rendering Engine Preview Generator\n";
	std::cout << "======================================================================\n\n";

	const int width = 640;
	const int height = 480;
	const size_t numPixels = width * height;

	PF_InData in_data;
	memset(&in_data, 0, sizeof(in_data));
	in_data.time_scale = 30000;
	in_data.current_time = 15000; // 0.5s into composition

	PF_OutData out_data;
	memset(&out_data, 0, sizeof(out_data));

	std::vector<PF_ParamDef> param_defs(VHS_NUM_PARAMS);
	std::vector<PF_ParamDef*> params(VHS_NUM_PARAMS);
	for (size_t i = 0; i < VHS_NUM_PARAMS; ++i) {
		memset(&param_defs[i], 0, sizeof(PF_ParamDef));
		params[i] = &param_defs[i];
	}

	// Create synthetic test input image (Color bars & gradient)
	std::vector<PF_Pixel> src_buffer(numPixels);
	int barWidth = width / 8;
	PF_Pixel colors[8] = {
		{255, 255, 255, 255}, {255, 255, 255, 0}, {255, 0, 255, 255}, {255, 0, 255, 0},
		{255, 255, 0, 255}, {255, 255, 0, 0}, {255, 0, 0, 255}, {255, 0, 0, 0}
	};
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			size_t idx = y * width + x;
			if (y < height / 2) {
				int barIdx = std::min(7, x / barWidth);
				src_buffer[idx] = colors[barIdx];
			} else {
				src_buffer[idx].alpha = 255;
				src_buffer[idx].red   = (A_u_char)(x * 255 / width);
				src_buffer[idx].green = (A_u_char)((y - height / 2) * 255 / (height / 2));
				src_buffer[idx].blue  = (A_u_char)((width - x) * 255 / width);
			}
		}
	}

	SavePPM("previews/original.ppm", src_buffer.data(), width, height);

	PF_EffectWorld input_layer;
	memset(&input_layer, 0, sizeof(input_layer));
	input_layer.width = width;
	input_layer.height = height;
	input_layer.rowbytes = width * sizeof(PF_Pixel);
	input_layer.data = src_buffer.data();
	params[VHS_INPUT]->u.ld = input_layer;

	std::vector<PF_Pixel> dst_buffer(numPixels);
	PF_EffectWorld output_layer;
	memset(&output_layer, 0, sizeof(output_layer));
	output_layer.width = width;
	output_layer.height = height;
	output_layer.rowbytes = width * sizeof(PF_Pixel);
	output_layer.data = dst_buffer.data();

	auto SetDefaultParameters = [&]() {
		params[VHS_MASTER_STRENGTH]->u.fs_d.value = 350.0;
		params[VHS_GLOBAL_ANIM_SPEED]->u.fs_d.value = 100.0;
		params[VHS_GLOBAL_SEED]->u.sd.value = 17;

		params[VHS_CAM_PROFILE]->u.pd.value = 1;
		params[VHS_CAM_STRENGTH]->u.fs_d.value = 100.0;
		params[VHS_CAM_EXPOSURE]->u.fs_d.value = 0.0;
		params[VHS_CAM_HIGHLIGHT_ROLLOFF]->u.fs_d.value = 50.0;
		params[VHS_CAM_SHADOW_LIFT]->u.fs_d.value = 10.0;
		params[VHS_CAM_COLOR_SAT]->u.fs_d.value = 100.0;
		params[VHS_CAM_WB_TEMP]->u.fs_d.value = 0.0;
		params[VHS_CAM_WB_TINT]->u.fs_d.value = 0.0;
		params[VHS_CAM_OPTICAL_SOFTNESS]->u.fs_d.value = 20.0;
		params[VHS_CAM_SHARPNESS]->u.fs_d.value = 30.0;
		params[VHS_CAM_LUMA_BANDWIDTH]->u.fs_d.value = 80.0;
		params[VHS_CAM_CHROMA_BANDWIDTH]->u.fs_d.value = 60.0;
		params[VHS_CAM_CHROMA_BLEED]->u.fs_d.value = 40.0;
		params[VHS_CAM_LOWLIGHT_NOISE]->u.fs_d.value = 15.0;
		params[VHS_CAM_FORMAT_RESP]->u.fs_d.value = 50.0;

		params[VHS_TAPE_MODE]->u.pd.value = 1;
		params[VHS_TAPE_STRENGTH]->u.fs_d.value = 50.0;
		params[VHS_TAPE_BAND_SIZE]->u.fs_d.value = 20.0;
		params[VHS_TAPE_SPEED]->u.fs_d.value = 60.0;
		params[VHS_TAPE_JITTER]->u.fs_d.value = 25.0;
		params[VHS_TAPE_DROPOUT_DENSITY]->u.fs_d.value = 15.0;
		params[VHS_TAPE_WRINKLE_AMOUNT]->u.fs_d.value = 10.0;
		params[VHS_TAPE_CHROMA_DEFORMATION]->u.fs_d.value = 30.0;

		params[VHS_GLITCH_ENABLE]->u.bd.value = FALSE;
		params[VHS_GLITCH_FREQ]->u.fs_d.value = 30.0;
		params[VHS_GLITCH_DURATION]->u.fs_d.value = 0.20;
		params[VHS_GLITCH_INTENSITY]->u.fs_d.value = 60.0;
		params[VHS_GLITCH_VARIATION]->u.fs_d.value = 40.0;
		params[VHS_GLITCH_RECOVERY]->u.fs_d.value = 0.15;
		params[VHS_GLITCH_TEARING]->u.fs_d.value = 50.0;
		params[VHS_GLITCH_BAND_DISRUPT]->u.fs_d.value = 35.0;
		params[VHS_GLITCH_TRACKING_FAIL]->u.fs_d.value = 25.0;
		params[VHS_GLITCH_CHROMA_INSTAB]->u.fs_d.value = 45.0;
		params[VHS_GLITCH_BURST_FLICKER]->u.fs_d.value = 20.0;
		params[VHS_GLITCH_SPEED]->u.fs_d.value = 100.0;
		params[VHS_GLITCH_SEED]->u.sd.value = 1337;

		params[VHS_FILTER_PRESET]->u.pd.value = 1; // Off
		params[VHS_FILTER_INTENSITY]->u.fs_d.value = 35.0;
		params[VHS_FILTER_WARMTH]->u.fs_d.value = 25.0;
		params[VHS_FILTER_GHOSTING]->u.fs_d.value = 20.0;
		params[VHS_FILTER_RINGING]->u.fs_d.value = 18.0;
		params[VHS_FILTER_BLACK_CRUSH]->u.fs_d.value = 15.0;

		params[VHS_GRAIN_ENABLE]->u.bd.value = FALSE;
		params[VHS_GRAIN_MODE]->u.pd.value = 1;
		params[VHS_GRAIN_AMOUNT]->u.fs_d.value = 25.0;
		params[VHS_GRAIN_LUMA_INTENSITY]->u.fs_d.value = 20.0;
		params[VHS_GRAIN_CHROMA_INTENSITY]->u.fs_d.value = 25.0;
		params[VHS_GRAIN_SIZE]->u.fs_d.value = 25.0;
		params[VHS_GRAIN_SOFTNESS]->u.fs_d.value = 30.0;
		params[VHS_GRAIN_SHADOW_RESP]->u.fs_d.value = 40.0;
		params[VHS_GRAIN_MIDTONE_RESP]->u.fs_d.value = 30.0;
		params[VHS_GRAIN_HIGHLIGHT_RESP]->u.fs_d.value = 15.0;
		params[VHS_GRAIN_MOTION_SPEED]->u.fs_d.value = 100.0;
		params[VHS_GRAIN_SEED]->u.sd.value = 42;

		params[VHS_TEXT_ENABLE]->u.bd.value = FALSE;
		params[VHS_TEXT_STYLE]->u.pd.value = 1;
		params[VHS_TEXT_YEAR]->u.sd.value = 1994;
		params[VHS_TEXT_MONTH]->u.sd.value = 5;
		params[VHS_TEXT_DAY]->u.sd.value = 18;
		params[VHS_TEXT_HOUR]->u.sd.value = 12;
		params[VHS_TEXT_MINUTE]->u.sd.value = 34;
		params[VHS_TEXT_PRESET_STRING]->u.pd.value = 1;
		params[VHS_TEXT_POS_X]->u.fs_d.value = 8.0;
		params[VHS_TEXT_POS_Y]->u.fs_d.value = 88.0;
		params[VHS_TEXT_SCALE]->u.fs_d.value = 100.0;
		params[VHS_TEXT_OPACITY]->u.fs_d.value = 90.0;
	};

	auto RenderMode = [&](const char *outPpm, std::function<void()> setupFn) {
		SetDefaultParameters();
		params[VHS_INPUT]->u.ld = input_layer;

		setupFn();

		memset(dst_buffer.data(), 0, dst_buffer.size() * sizeof(PF_Pixel));
		EffectMain(PF_Cmd_RENDER, &in_data, &out_data, params.data(), &output_layer, nullptr);
		SavePPM(outPpm, dst_buffer.data(), width, height);
		std::cout << "Rendered native C++ preview: " << outPpm << "\n";
	};

	RenderMode("previews/camera_fxc6.ppm", [&]() {
		params[VHS_CAM_PROFILE]->u.pd.value = 1;
		params[VHS_CAM_STRENGTH]->u.fs_d.value = 100.0;
		params[VHS_CAM_EXPOSURE]->u.fs_d.value = 10.0;
	});

	RenderMode("previews/camera_gen7.ppm", [&]() {
		params[VHS_CAM_PROFILE]->u.pd.value = 2;
		params[VHS_CAM_STRENGTH]->u.fs_d.value = 100.0;
		params[VHS_CAM_SHADOW_LIFT]->u.fs_d.value = 30.0;
	});

	RenderMode("previews/camera_jvc_c7u.ppm", [&]() {
		params[VHS_CAM_PROFILE]->u.pd.value = 3;
		params[VHS_CAM_STRENGTH]->u.fs_d.value = 100.0;
		params[VHS_CAM_HIGHLIGHT_ROLLOFF]->u.fs_d.value = 80.0;
	});

	RenderMode("previews/vhs_sp_mode.ppm", [&]() {
		params[VHS_TAPE_MODE]->u.pd.value = 1;
		params[VHS_TAPE_STRENGTH]->u.fs_d.value = 80.0;
		params[VHS_TAPE_JITTER]->u.fs_d.value = 40.0;
		params[VHS_CAM_CHROMA_BLEED]->u.fs_d.value = 60.0;
	});

	RenderMode("previews/vhs_lp_mode.ppm", [&]() {
		params[VHS_TAPE_MODE]->u.pd.value = 2;
		params[VHS_TAPE_STRENGTH]->u.fs_d.value = 100.0;
		params[VHS_TAPE_JITTER]->u.fs_d.value = 80.0;
		params[VHS_TAPE_BAND_SIZE]->u.fs_d.value = 40.0;
		params[VHS_TAPE_DROPOUT_DENSITY]->u.fs_d.value = 50.0;
	});

	RenderMode("previews/bad_signal_glitch.ppm", [&]() {
		params[VHS_GLITCH_ENABLE]->u.bd.value = TRUE;
		params[VHS_GLITCH_FREQ]->u.fs_d.value = 100.0;
		params[VHS_GLITCH_DURATION]->u.fs_d.value = 1.0;
		params[VHS_GLITCH_INTENSITY]->u.fs_d.value = 100.0;
		params[VHS_GLITCH_TEARING]->u.fs_d.value = 90.0;
		params[VHS_GLITCH_BAND_DISRUPT]->u.fs_d.value = 80.0;
	});

	RenderMode("previews/crt_tv_trinitron.ppm", [&]() {
		params[VHS_FILTER_PRESET]->u.pd.value = 2;
		params[VHS_FILTER_INTENSITY]->u.fs_d.value = 60.0;
		params[VHS_FILTER_WARMTH]->u.fs_d.value = 40.0;
		params[VHS_FILTER_GHOSTING]->u.fs_d.value = 35.0;
		params[VHS_FILTER_RINGING]->u.fs_d.value = 30.0;
	});

	RenderMode("previews/grain_color.ppm", [&]() {
		params[VHS_GRAIN_ENABLE]->u.bd.value = TRUE;
		params[VHS_GRAIN_MODE]->u.pd.value = 2;
		params[VHS_GRAIN_AMOUNT]->u.fs_d.value = 80.0;
		params[VHS_GRAIN_SIZE]->u.fs_d.value = 25.0;
		params[VHS_GRAIN_SHADOW_RESP]->u.fs_d.value = 60.0;
	});

	RenderMode("previews/date_text_osd.ppm", [&]() {
		params[VHS_TEXT_ENABLE]->u.bd.value = TRUE;
		params[VHS_TEXT_STYLE]->u.pd.value = 2;
		params[VHS_TEXT_YEAR]->u.sd.value = 1994;
		params[VHS_TEXT_MONTH]->u.sd.value = 5;
		params[VHS_TEXT_DAY]->u.sd.value = 18;
		params[VHS_TEXT_HOUR]->u.sd.value = 12;
		params[VHS_TEXT_MINUTE]->u.sd.value = 34;
		params[VHS_TEXT_SCALE]->u.fs_d.value = 120.0;
		params[VHS_TEXT_OPACITY]->u.fs_d.value = 100.0;
	});

	RenderMode("previews/vhs_master_full_suite.ppm", [&]() {
		params[VHS_CAM_PROFILE]->u.pd.value = 3;
		params[VHS_TAPE_MODE]->u.pd.value = 1;
		params[VHS_GLITCH_ENABLE]->u.bd.value = TRUE;
		params[VHS_GLITCH_INTENSITY]->u.fs_d.value = 80.0;
		params[VHS_FILTER_PRESET]->u.pd.value = 2;
		params[VHS_GRAIN_ENABLE]->u.bd.value = TRUE;
		params[VHS_GRAIN_AMOUNT]->u.fs_d.value = 40.0;
		params[VHS_TEXT_ENABLE]->u.bd.value = TRUE;
		params[VHS_TEXT_STYLE]->u.pd.value = 2;
	});

	return 0;
}
