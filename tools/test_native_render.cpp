#include <iostream>
#include <vector>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <functional>
#include <algorithm>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

typedef unsigned char A_u_char;
typedef unsigned short A_u_short;
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
#define DllExport

typedef struct {
	A_u_char alpha, red, green, blue;
} PF_Pixel8;

typedef struct {
	int width;
	int height;
	int rowbytes;
	void *data;
} PF_LayerDef;

typedef struct {
	int time_scale;
	int current_time;
} PF_InData;

typedef struct {
	int my_version;
	int out_flags;
	int out_flags2;
	int num_params;
	char return_msg[512];
} PF_OutData;

typedef struct {
	union {
		struct { int value; } bd;
		struct { double value; } fs_d;
		struct { int value; } pd;
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
	struct { int num_channels; } fi;
} PF_SoundWorld;

// Stub out AFX and AE_Effect.h macros for standalone testing
#define _H_AEConfig
#define _H_AE_Effect
#define _H_AE_EffectCB
#define PF_SPRINTF(...) (0)
#define AEFX_CLR_STRUCT(s) memset(&(s), 0, sizeof(s))
#define PF_ADD_PARAM(in_data, id, def) PF_Err_NONE
#define PF_ADD_TOPIC(NAME, ID) PF_Err_NONE
#define PF_END_TOPIC(ID) PF_Err_NONE
#define PF_ADD_FLOAT_SLIDERX(NAME, V_MIN, V_MAX, S_MIN, S_MAX, DFLT, PREC, DISP, FLAGS, ID) PF_Err_NONE
#define PF_ADD_CHECKBOXX(NAME, DESC, DFLT, FLAGS, ID) PF_Err_NONE
#define PF_ADD_POPUP(NAME, NUM, DFLT, CHOICES, ID) PF_Err_NONE
#define PF_ADD_LAYER(NAME, DFLT, ID) PF_Err_NONE
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

struct PixelStats {
	size_t changedPixels;
	double meanAbsError;
	int minR, maxR;
	int minG, maxG;
	int minB, maxB;
};

static PixelStats CompareImages(const PF_Pixel8 *src, const PF_Pixel8 *dst, int width, int height) {
	PixelStats stats = {0, 0.0, 255, 0, 255, 0, 255, 0};
	size_t total = width * height;
	double totalDiff = 0.0;

	for (size_t i = 0; i < total; ++i) {
		int dr = std::abs((int)dst[i].red - (int)src[i].red);
		int dg = std::abs((int)dst[i].green - (int)src[i].green);
		int db = std::abs((int)dst[i].blue - (int)src[i].blue);

		if (dr > 0 || dg > 0 || db > 0) {
			stats.changedPixels++;
		}
		totalDiff += (dr + dg + db) / 3.0;

		stats.minR = std::min(stats.minR, (int)dst[i].red);
		stats.maxR = std::max(stats.maxR, (int)dst[i].red);
		stats.minG = std::min(stats.minG, (int)dst[i].green);
		stats.maxG = std::max(stats.maxG, (int)dst[i].green);
		stats.minB = std::min(stats.minB, (int)dst[i].blue);
		stats.maxB = std::max(stats.maxB, (int)dst[i].blue);
	}

	stats.meanAbsError = totalDiff / (double)total;
	return stats;
}

int main() {
	std::cout << "======================================================================\n";
	std::cout << "VHS6RIFT Production Native C++ Render Pipeline Test Suite\n";
	std::cout << "======================================================================\n\n";

	const int width = 256;
	const int height = 256;
	const size_t numPixels = width * height;

	// Prepare mock InData & OutData
	PF_InData in_data;
	memset(&in_data, 0, sizeof(in_data));
	in_data.time_scale = 30000;
	in_data.current_time = 0;

	PF_OutData out_data;
	memset(&out_data, 0, sizeof(out_data));

	// Allocate and setup parameters array
	std::vector<PF_ParamDef> param_defs(VHS6RIFT_NUM_PARAMS);
	std::vector<PF_ParamDef*> params(VHS6RIFT_NUM_PARAMS);
	for (size_t i = 0; i < VHS6RIFT_NUM_PARAMS; ++i) {
		memset(&param_defs[i], 0, sizeof(PF_ParamDef));
		params[i] = &param_defs[i];
	}

	// Set default parameter values for test suite matching ParamsSetup
	params[VHS6RIFT_MAIN_ENABLE]->u.bd.value = TRUE;
	params[VHS6RIFT_MASTER_STRENGTH]->u.fs_d.value = 100.0;
	params[VHS6RIFT_SIGNAL_DEGRADATION]->u.fs_d.value = 30.0;
	params[VHS6RIFT_LUMA_SOFTNESS]->u.fs_d.value = 25.0;
	params[VHS6RIFT_CHROMA_DEGRADATION]->u.fs_d.value = 30.0;
	params[VHS6RIFT_CHROMA_BLEED]->u.fs_d.value = 35.0;
	params[VHS6RIFT_CHROMA_OFFSET_X]->u.fs_d.value = 3.0;
	params[VHS6RIFT_CHROMA_OFFSET_Y]->u.fs_d.value = 0.0;
	params[VHS6RIFT_EDGE_RESPONSE]->u.fs_d.value = 25.0;
	params[VHS6RIFT_CONTRAST]->u.fs_d.value = 8.0;
	params[VHS6RIFT_SATURATION]->u.fs_d.value = -10.0;
	params[VHS6RIFT_BLACK_LEVEL]->u.fs_d.value = 4.0;
	params[VHS6RIFT_ANALOG_COLOR_SHIFT]->u.fs_d.value = 0.0;
	params[VHS6RIFT_SIGNAL_INSTABILITY]->u.fs_d.value = 20.0;
	params[VHS6RIFT_IMAGE_WARPING]->u.fs_d.value = 15.0;
	params[VHS6RIFT_GLOBAL_SEED]->u.fs_d.value = 1337.0;

	// Create synthetic source image buffer (radial color gradient)
	std::vector<PF_Pixel8> src_buffer(numPixels);
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			size_t idx = y * width + x;
			src_buffer[idx].red   = (A_u_char)(x * 255 / width);
			src_buffer[idx].green = (A_u_char)(y * 255 / height);
			src_buffer[idx].blue  = (A_u_char)((x + y) * 255 / (width + height));
			src_buffer[idx].alpha = 255;
		}
	}

	// Setup Input LayerDef in params[0]
	PF_LayerDef input_layer;
	memset(&input_layer, 0, sizeof(input_layer));
	input_layer.width = width;
	input_layer.height = height;
	input_layer.rowbytes = width * sizeof(PF_Pixel8);
	input_layer.data = src_buffer.data();
	params[VHS6RIFT_INPUT]->u.ld = input_layer;

	// Setup Output LayerDef
	std::vector<PF_Pixel8> dst_buffer(numPixels);
	PF_LayerDef output_layer;
	memset(&output_layer, 0, sizeof(output_layer));
	output_layer.width = width;
	output_layer.height = height;
	output_layer.rowbytes = width * sizeof(PF_Pixel8);
	output_layer.data = dst_buffer.data();

	int passedTests = 0;
	int totalTests = 0;

	// Test 1: Diagnostic Mode (Solid Magenta Verification)
	{
		totalTests++;
		params[VHS6RIFT_DIAGNOSTIC_MODE]->u.bd.value = TRUE;
		memset(dst_buffer.data(), 0, dst_buffer.size() * sizeof(PF_Pixel8));

		EffectMain(PF_Cmd_RENDER, &in_data, &out_data, params.data(), &output_layer, nullptr);
		PixelStats stats = CompareImages(src_buffer.data(), dst_buffer.data(), width, height);

		bool isAllMagenta = true;
		for (size_t i = 0; i < numPixels; ++i) {
			if (dst_buffer[i].red != 255 || dst_buffer[i].green != 0 || dst_buffer[i].blue != 255) {
				isAllMagenta = false;
				break;
			}
		}

		if (isAllMagenta && stats.changedPixels == numPixels) {
			std::cout << "  [PASS] Test 1: Diagnostic Mode (Outputs 100% Solid Magenta RGB: 255,0,255).\n";
			passedTests++;
		} else {
			std::cout << "  [FAIL] Test 1: Diagnostic Mode failed.\n";
		}

		// Turn off diagnostic mode
		params[VHS6RIFT_DIAGNOSTIC_MODE]->u.bd.value = FALSE;
	}

	// Helper test runner lambda
	auto RunTestCase = [&](const std::string &name, std::function<void()> setupFn) {
		totalTests++;
		// Reset all enable checkboxes to FALSE
		params[VHS6RIFT_MAIN_ENABLE]->u.bd.value = FALSE;
		params[VHS6RIFT_TAPE_ENABLE]->u.bd.value = FALSE;
		params[VHS6RIFT_TRACKING_ENABLE]->u.bd.value = FALSE;
		params[VHS6RIFT_GRAIN_ENABLE]->u.bd.value = FALSE;
		params[VHS6RIFT_CRT_ENABLE]->u.bd.value = FALSE;
		params[VHS6RIFT_CAMERA_ENABLE]->u.bd.value = FALSE;
		params[VHS6RIFT_MODEL_ENABLE]->u.bd.value = FALSE;
		params[VHS6RIFT_MODES_ENABLE]->u.bd.value = FALSE;
		params[VHS6RIFT_OTHER_ENABLE]->u.bd.value = FALSE;

		setupFn();

		memset(dst_buffer.data(), 0, dst_buffer.size() * sizeof(PF_Pixel8));
		EffectMain(PF_Cmd_RENDER, &in_data, &out_data, params.data(), &output_layer, nullptr);

		PixelStats stats = CompareImages(src_buffer.data(), dst_buffer.data(), width, height);

		std::cout << "  " << std::left << std::setw(38) << name
		          << " -> Changed: " << std::setw(6) << stats.changedPixels << "/" << numPixels
		          << " | MAE: " << std::fixed << std::setprecision(2) << std::setw(5) << stats.meanAbsError
		          << " | Output Range R:[" << std::setw(3) << stats.minR << ".." << std::setw(3) << stats.maxR << "]"
		          << " G:[" << std::setw(3) << stats.minG << ".." << std::setw(3) << stats.maxG << "]"
		          << " B:[" << std::setw(3) << stats.minB << ".." << std::setw(3) << stats.maxB << "]\n";

		if (stats.changedPixels > 0 && stats.meanAbsError > 0.1) {
			passedTests++;
		} else {
			std::cout << "    [FAIL] No pixels changed or MAE zero!\n";
		}
	};

	std::cout << "\n--- Production Render Pass Individual Module Tests ---\n";

	RunTestCase("Test 2: Main VHS Engine (Default)", [&]() {
		params[VHS6RIFT_MAIN_ENABLE]->u.bd.value = TRUE;
	});

	RunTestCase("Test 3: Signal Instability", [&]() {
		params[VHS6RIFT_MAIN_ENABLE]->u.bd.value = TRUE;
		params[VHS6RIFT_SIGNAL_INSTABILITY]->u.fs_d.value = 50.0;
	});

	RunTestCase("Test 4: Image Warping", [&]() {
		params[VHS6RIFT_MAIN_ENABLE]->u.bd.value = TRUE;
		params[VHS6RIFT_IMAGE_WARPING]->u.fs_d.value = 50.0;
	});

	RunTestCase("Test 5: Tape Effects (Tape Band)", [&]() {
		params[VHS6RIFT_TAPE_ENABLE]->u.bd.value = TRUE;
		params[VHS6RIFT_TAPE_TYPE]->u.pd.value = 2; // Horizontal Tape Band
		params[VHS6RIFT_TAPE_STRENGTH]->u.fs_d.value = 80.0;
		params[VHS6RIFT_TAPE_WIDTH]->u.fs_d.value = 40.0;
	});

	RunTestCase("Test 6: Tracking Damage (uni.vhs)", [&]() {
		params[VHS6RIFT_TRACKING_ENABLE]->u.bd.value = TRUE;
		params[VHS6RIFT_TRACKING_BAND_STRENGTH]->u.fs_d.value = 50.0;
		params[VHS6RIFT_TRACKING_HEAD_SWITCHING]->u.fs_d.value = 50.0;
	});

	RunTestCase("Test 7: CRT TV Engine", [&]() {
		params[VHS6RIFT_CRT_ENABLE]->u.bd.value = TRUE;
		params[VHS6RIFT_CRT_SCANLINE_STRENGTH]->u.fs_d.value = 40.0;
		params[VHS6RIFT_CRT_CURVATURE]->u.fs_d.value = 20.0;
	});

	RunTestCase("Test 8: Camera Profile (Handycam)", [&]() {
		params[VHS6RIFT_CAMERA_ENABLE]->u.bd.value = TRUE;
		params[VHS6RIFT_CAMERA_PROFILE]->u.pd.value = 4; // Sony Handycam
		params[VHS6RIFT_CAMERA_STRENGTH]->u.fs_d.value = 100.0;
	});

	RunTestCase("Test 9: Reference Model 1", [&]() {
		params[VHS6RIFT_MODEL_ENABLE]->u.bd.value = TRUE;
		params[VHS6RIFT_MODEL_SELECTOR]->u.pd.value = 1; // Model 1
		params[VHS6RIFT_MODEL_INTENSITY]->u.fs_d.value = 80.0;
		params[VHS6RIFT_MODEL_BLEND]->u.fs_d.value = 100.0;
	});

	RunTestCase("Test 10: Additional VHS Mode (NTSC-RS)", [&]() {
		params[VHS6RIFT_MODES_ENABLE]->u.bd.value = TRUE;
		params[VHS6RIFT_MODES_SELECTOR]->u.pd.value = 5; // NTSC-RS
		params[VHS6RIFT_MODES_DRIVE]->u.fs_d.value = 50.0;
	});

	std::cout << "\n======================================================================\n";
	std::cout << "Summary: " << passedTests << " / " << totalTests << " tests PASSED.\n";
	std::cout << "======================================================================\n";

	return (passedTests == totalTests) ? 0 : 1;
}
