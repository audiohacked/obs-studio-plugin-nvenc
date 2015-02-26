project(obs-nvenc)

set(obs-nvenc_HEADERS
	obs-nvenc.h
	nvEncodeAPI.h)

set(obs-nvenc_SOURCES
	obs-nvenc.c
	obs-nvenc-helper.c
	obs-nvenc-optional.c
	obs-nvenc-plugin-main.c)

add_library(obs-nvenc MODULE
	${obs-nvenc_HEADERS}
	${obs-nvenc_SOURCES})
target_link_libraries(obs-nvenc
	libobs)

install_obs_plugin_with_data(obs-nvenc data)
