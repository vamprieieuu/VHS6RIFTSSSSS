#pragma once
#ifndef PARAM_UTILS_H
#define PARAM_UTILS_H

#include "AE_Effect.h"
#include "AE_Macros.h"
#include "String_Utils.h"

#define PF_ADD_SLIDER(NAME, MIN_VAL, MAX_VAL, SLD_MIN, SLD_MAX, DFLT, ID) \
	do { \
		AEFX_CLR_STRUCT(def); \
		def.param_type = PF_Param_SLIDER; \
		PF_STRCPY(def.name, (NAME)); \
		def.u.sd.valid_min = static_cast<A_long>(MIN_VAL); \
		def.u.sd.valid_max = static_cast<A_long>(MAX_VAL); \
		def.u.sd.slider_min = static_cast<A_long>(SLD_MIN); \
		def.u.sd.slider_max = static_cast<A_long>(SLD_MAX); \
		def.u.sd.value = static_cast<A_long>(DFLT); \
		def.u.sd.dephault = static_cast<A_long>(DFLT); \
		def.uu.id = (ID); \
		err = PF_ADD_PARAM(in_data, -1, &def); \
		if (err) return err; \
	} while (0)

#define PF_ADD_FLOAT_SLIDER(NAME, MIN_VAL, MAX_VAL, SLD_MIN, SLD_MAX, DFLT, PREC, ID) \
	do { \
		AEFX_CLR_STRUCT(def); \
		def.param_type = PF_Param_FLOAT_SLIDER; \
		PF_STRCPY(def.name, (NAME)); \
		def.u.fs_d.valid_min = static_cast<PF_FpShort>(MIN_VAL); \
		def.u.fs_d.valid_max = static_cast<PF_FpShort>(MAX_VAL); \
		def.u.fs_d.slider_min = static_cast<PF_FpShort>(SLD_MIN); \
		def.u.fs_d.slider_max = static_cast<PF_FpShort>(SLD_MAX); \
		def.u.fs_d.value = static_cast<PF_FpLong>(DFLT); \
		def.u.fs_d.dephault = static_cast<PF_FpShort>(DFLT); \
		def.u.fs_d.precision = static_cast<A_short>(PREC); \
		def.u.fs_d.display_flags = 0; \
		def.u.fs_d.fs_flags = 0; \
		def.uu.id = (ID); \
		err = PF_ADD_PARAM(in_data, -1, &def); \
		if (err) return err; \
	} while (0)

#define PF_ADD_CHECKBOX(NAME, STR_ON, DFLT, FLAGS, ID) \
	do { \
		AEFX_CLR_STRUCT(def); \
		def.param_type = PF_Param_CHECKBOX; \
		PF_STRCPY(def.name, (NAME)); \
		def.u.bd.u.nameptr = (STR_ON); \
		def.u.bd.value = static_cast<PF_Boolean>(DFLT); \
		def.u.bd.dephault = static_cast<PF_Boolean>(DFLT); \
		def.flags = (FLAGS); \
		def.uu.id = (ID); \
		err = PF_ADD_PARAM(in_data, -1, &def); \
		if (err) return err; \
	} while (0)

#define PF_ADD_POPUP(NAME, NUM_CHOICES, DFLT, CHOICES_STR, ID) \
	do { \
		AEFX_CLR_STRUCT(def); \
		def.param_type = PF_Param_POPUP; \
		PF_STRCPY(def.name, (NAME)); \
		def.u.pd.num_choices = static_cast<A_short>(NUM_CHOICES); \
		def.u.pd.value = static_cast<A_short>(DFLT); \
		def.u.pd.dephault = static_cast<A_short>(DFLT); \
		def.u.pd.u.namesptr = (CHOICES_STR); \
		def.uu.id = (ID); \
		err = PF_ADD_PARAM(in_data, -1, &def); \
		if (err) return err; \
	} while (0)

#define PF_ADD_LAYER(NAME, DFLT, ID) \
	do { \
		AEFX_CLR_STRUCT(def); \
		def.param_type = PF_Param_LAYER; \
		PF_STRCPY(def.name, (NAME)); \
		def.u.ld.dephault = static_cast<A_long>(DFLT); \
		def.uu.id = (ID); \
		err = PF_ADD_PARAM(in_data, -1, &def); \
		if (err) return err; \
	} while (0)

#define PF_ADD_TOPIC(NAME, ID) \
	do { \
		AEFX_CLR_STRUCT(def); \
		def.param_type = PF_Param_GROUP_START; \
		PF_STRCPY(def.name, (NAME)); \
		def.flags = PF_ParamFlag_START_COLLAPSED; \
		def.uu.id = (ID); \
		err = PF_ADD_PARAM(in_data, -1, &def); \
		if (err) return err; \
	} while (0)

#define PF_END_TOPIC(ID) \
	do { \
		AEFX_CLR_STRUCT(def); \
		def.param_type = PF_Param_GROUP_END; \
		def.uu.id = (ID); \
		err = PF_ADD_PARAM(in_data, -1, &def); \
		if (err) return err; \
	} while (0)

#endif // PARAM_UTILS_H
