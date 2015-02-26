#include <obs-module.h>
#include <util/platform.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-nvenc", "en-US")

#if defined (_WIN64)
#define NVIDIA_LIB "nvEncodeAPI64.dll"
#endif

#if defined (_WIN32) && !defined(_WIN64)
#define NVIDIA_LIB "nvEncodeAPI.dll"
#endif

#if !defined (_WIN32) && !defined(_WIN64)
#define NVIDIA_LIB "libnvidia-encode.so"
#endif

extern struct obs_encoder_info obs_nvenc_encoder;
void *obs_nvenc_lib = NULL;

bool obs_module_load(void)
{
	obs_nvenc_lib = os_dlopen(NVIDIA_LIB);
	if (obs_nvenc_lib == NULL) {
		blog(LOG_ERROR,
			"[obs-nvenc] ERROR: nVidia Encoder DLL missing! Looking for %s",
			NVIDIA_LIB);
			return true;
	}

	obs_register_encoder(&obs_nvenc_encoder);
	return true;
}
