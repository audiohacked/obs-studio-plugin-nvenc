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

#include <stdio.h>
#include <util/dstr.h>
#include <util/darray.h>
#include <util/platform.h>
#include <obs-module.h>

#include "obs-nvenc.h"
#include "nvEncodeAPI.h"

/* ------------------------------------------------------------------------- */

static const char *obs_nvenc_getname(void)
{
	return "NVENC";
}

static void *obs_nvenc_create(obs_data_t *settings, obs_encoder_t *encoder)
{
	struct obs_nvenc *obsnv = bzalloc(sizeof(struct obs_nvenc));
	obsnv->encoder = encoder;
	obsnv->settings = settings;

	obs_nvenc_helper_create_instance(obsnv);
	obs_nvenc_helper_open_session(obsnv);
	obs_nvenc_helper_select_codec(obsnv);
	obs_nvenc_helper_get_preset(obsnv);
	obs_nvenc_helper_init_encoder(obsnv);
	obs_nvenc_helper_create_buffer(obsnv);

	return obsnv;
}

static void obs_nvenc_destroy(void *data)
{
	struct obs_nvenc *obsnv = data;

	memset(&obsnv->nvenc_picture, 0, sizeof(obsnv->nvenc_picture));
	obsnv->nvenc_picture.encodePicFlags = NV_ENC_PIC_FLAG_EOS;
	obsnv->api->nvEncEncodePicture(obsnv->nvenc_device, &obsnv->nvenc_picture);

	if (obsnv) {
		os_end_high_performance(obsnv->performance_token);
		clear_data(obsnv);
		da_free(obsnv->packet_data);
		bfree(obsnv);
	}
}

static bool obs_nvenc_encode(void *data, struct encoder_frame *frame,
		struct encoder_packet *packet, bool *received_packet)
{
	struct obs_nvenc *obsnv = data;
	NVENCSTATUS nvStatus = NV_ENC_ERR_GENERIC;

	if (!frame || !packet || !received_packet)
		return false;

	// initialize input buffer lock data structure
	NV_ENC_LOCK_INPUT_BUFFER input_buffer_lock;
	memset(&input_buffer_lock, 0, sizeof(NV_ENC_LOCK_INPUT_BUFFER));
	SET_VER(input_buffer_lock, NV_ENC_LOCK_INPUT_BUFFER);

	//initialize output bitstream buffer lock data structure
	NV_ENC_LOCK_BITSTREAM output_buffer_lock;
	memset(&output_buffer_lock, 0, sizeof(NV_ENC_LOCK_BITSTREAM));
	SET_VER(output_buffer_lock, NV_ENC_LOCK_BITSTREAM);

	//lock input buffer
	input_buffer_lock.inputBuffer = &obsnv->nvenc_buffer_input;
	obsnv->api->nvEncLockInputBuffer(obsnv->nvenc_device, &input_buffer_lock);

	//fill data
	obs_nvenc_helper_fill_frame(obsnv, frame);

	//unlock input buffer
	obsnv->api->nvEncUnlockInputBuffer(obsnv->nvenc_device,
			obsnv->nvenc_buffer_input.inputBuffer);

	// maybe set Per-Frame Encode parameters

	// submit buffer for encoding
	obsnv->nvenc_picture.inputBuffer = &obsnv->nvenc_buffer_input;
	nvStatus = obsnv->api->nvEncEncodePicture(obsnv->nvenc_device,
			&obsnv->nvenc_picture);
	if (nvStatus != NV_ENC_SUCCESS) {
		warn("encode failed");
		return false;
	}

	//lock output bitstream buffer
	output_buffer_lock.bitstreamBufferPtr = &obsnv->nvenc_buffer_output;
	obsnv->api->nvEncLockBitstream(obsnv->nvenc_device, &output_buffer_lock);

	//grab data
	obs_nvenc_helper_save_bitstream(obsnv, packet);

	//unlock output bitstream buffer
	obsnv->api->nvEncUnlockBitstream(obsnv->nvenc_device,
			obsnv->nvenc_buffer_output.bitstreamBuffer);

	return false;
}

struct obs_encoder_info obs_nvenc_encoder = {
	.id             = "obs_nvenc",
	.type           = OBS_ENCODER_VIDEO,
	.codec          = "h264",
	.get_name       = obs_nvenc_getname,
	.create         = obs_nvenc_create,
	.destroy        = obs_nvenc_destroy,
	.encode         = obs_nvenc_encode,
	.update         = obs_nvenc_update,
	.get_properties = obs_nvenc_get_properties,
	.get_defaults   = obs_nvenc_get_defaults,
	.get_extra_data = obs_nvenc_get_extra_data,
	.get_sei_data   = obs_nvenc_get_sei_data,
	.get_video_info = obs_nvenc_get_video_info
};
