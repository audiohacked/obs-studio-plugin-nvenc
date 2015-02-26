/******************************************************************************
    Copyright (C) 2015 by Sean Nelson <audiohacked@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/
#pragma once

#include "nvEncodeAPI.h"

#define do_log(level, format, ...) \
	blog(level, "[nvenc encoder: '%s'] " format, \
			obs_encoder_get_name(obsnv->encoder), ##__VA_ARGS__)

#define error(format, ...) do_log(LOG_ERROR,   format, ##__VA_ARGS__)
#define warn(format, ...)  do_log(LOG_WARNING, format, ##__VA_ARGS__)
#define info(format, ...)  do_log(LOG_INFO,    format, ##__VA_ARGS__)
#define debug(format, ...) do_log(LOG_DEBUG,   format, ##__VA_ARGS__)

#define SET_VER(configStruct, type) {configStruct.version = type##_VER;}

extern void *obs_nvenc_lib;

typedef enum _OBS_NVENC_PROFILE {
	OBS_NVENC_PROFILE_AUTOSELECT                   = 1, //NV_ENC_CODEC_PROFILE_AUTOSELECT_GUID
	OBS_NVENC_PROFILE_H264_BASELINE                = 2, //NV_ENC_H264_PROFILE_BASELINE_GUID
	OBS_NVENC_PROFILE_H264_MAIN                    = 3, //NV_ENC_H264_PROFILE_MAIN_GUID
	OBS_NVENC_PROFILE_H264_HIGH                    = 4, //NV_ENC_H264_PROFILE_HIGH_GUID
	OBS_NVENC_PROFILE_H264_HIGH_444                = 5, //NV_ENC_H264_PROFILE_HIGH_444_GUID
	OBS_NVENC_PROFILE_H264_STEREO                  = 6, //NV_ENC_H264_PROFILE_STEREO_GUID
	OBS_NVENC_PROFILE_H264_SVC_TEMPORAL_SCALABILTY = 7, //NV_ENC_H264_PROFILE_SVC_TEMPORAL_SCALABILTY
	OBS_NVENC_PROFILE_H264_CONSTRAINED_HIGH        = 8, //NV_ENC_H264_PROFILE_CONSTRAINED_HIGH_GUID
	OBS_NVENC_PROFILE_H265_MAIN                    = 9, //NV_ENC_HEVC_PROFILE_MAIN_GUID
} OBS_NVENC_PROFILE;

typedef enum _OBS_NVENC_PRESET {
	OBS_NVENC_PRESET_DEFAULT        = 1, //NV_ENC_PRESET_DEFAULT_GUID
	OBS_NVENC_PRESET_HP             = 2, //NV_ENC_PRESET_HP_GUID
	OBS_NVENC_PRESET_HQ             = 3, //NV_ENC_PRESET_HQ_GUID
	OBS_NVENC_PRESET_BD             = 4, //NV_ENC_PRESET_BD_GUID
	OBS_NVENC_PRESET_LOW_LATENCY    = 5, //NV_ENC_PRESET_LOW_LATENCY_DEFAULT_GUID
	OBS_NVENC_PRESET_LOW_LATENCY_HQ = 6, //NV_ENC_PRESET_LOW_LATENCY_HQ_GUID
	OBS_NVENC_PRESET_LOW_LATENCY_HP = 7, //NV_ENC_PRESET_LOW_LATENCY_HP_GUID
	OBS_NVENC_PRESET_LOSSLESS       = 8, //NV_ENC_PRESET_LOSSLESS_DEFAULT_GUID
	OBS_NVENC_PRESET_LOSSLESS_HP    = 9, //NV_ENC_PRESET_LOSSLESS_HP_GUID
} OBS_NVENC_PRESET;

/* ------------------------------------------------------------------------- */

struct obs_nvenc {
	obs_encoder_t                  *encoder;
	obs_data_t                     *settings;
	NV_ENCODE_API_FUNCTION_LIST*   api;
	void                           *nvenc_device;

	GUID                           nvenc_codec;
	GUID                           nvenc_preset;
	GUID                           nvenc_profile;

	NV_ENC_CREATE_INPUT_BUFFER     nvenc_buffer_input;
	NV_ENC_CREATE_BITSTREAM_BUFFER nvenc_buffer_output;
	NV_ENC_PIC_PARAMS              nvenc_picture;
	
	DARRAY(uint8_t)                packet_data;

	uint8_t                        *extra_data;
	uint8_t                        *sei;

	size_t                         extra_data_size;
	size_t                         sei_size;

	os_performance_token_t         *performance_token;
};

/* ------------------------------------------------------------------------- */

extern void clear_data(struct obs_nvenc *obsnv);
extern void obs_nvenc_helper_fill_frame(struct obs_nvenc *obsnv,
		struct encoder_frame *frame);
extern void obs_nvenc_helper_save_bitstream(struct obs_nvenc *obsnv,
		struct encoder_packet *packet);
extern const char* obs_nvenc_profile_string(OBS_NVENC_PROFILE profile);
extern const char* obs_nvenc_preset_string(OBS_NVENC_PRESET preset);

/* ------------------------------------------------------------------------- */
//Create API Instance from library
extern NVENCSTATUS obs_nvenc_helper_create_instance(void *data);

//Open Encode Session with NvEncOpenEncodeSessionEx
//	Initialize the Encode Device with
//		NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS::device
//		NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS::deviceType
//			= hasDX ? NV_ENC_DEVICE_TYPE_DIRECTX : NV_ENC_DEVICE_TYPE_CUDA;
extern NVENCSTATUS obs_nvenc_helper_open_session(void* data);

//Select codec by
//		getting codec count with NvEncGetEncodeGUIDCount
//		allocate buffer for all GUIDs
//		populate buffer with all GUIDs using NvEncGetEncodeGUIDs
extern NVENCSTATUS obs_nvenc_helper_select_codec(void *data);

//Query Capabilities Values
//		NV_ENC_CAPS_PARAM::capsToQuery
//		NvEncGetEncoderCaps
extern NVENCSTATUS obs_nvenc_helper_query_caps(void *data);

//Get Preset Encoder Configurations
extern NVENCSTATUS obs_nvenc_helper_get_preset(void *data);

//Select Encoder Profile
extern NVENCSTATUS obs_nvenc_helper_set_profile(void *data);

//Get Supported Input Format List
extern NVENCSTATUS obs_nvenc_helper_get_input_formats(void *data);

//Initialize HW Encoder Session
//	Configure Encode Session Attributes
//	Finalize the Codec Configuration for Encoding
//	Setting Encode Session Attributes
extern NVENCSTATUS obs_nvenc_helper_init_encoder(void *data);

//Create Resources to hold I/O Data
extern NVENCSTATUS obs_nvenc_helper_create_buffer(void *data);

/* ------------------------------------------------------------------------- */

extern void obs_nvenc_get_defaults(obs_data_t *settings);
extern obs_properties_t *obs_nvenc_get_properties(void *unused);
extern bool obs_nvenc_update(void *data, obs_data_t *settings);
extern bool obs_nvenc_get_extra_data(void *data, uint8_t **extra_data, size_t *size);
extern bool obs_nvenc_get_sei_data(void *data, uint8_t **sei, size_t *size);
extern bool obs_nvenc_get_video_info(void *data, struct video_scale_info *info);

/* ------------------------------------------------------------------------- */