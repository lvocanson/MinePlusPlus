include(FetchContent)

# Override dependencies options
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

set(MINIFB_USE_OPENGL_API     OFF)
set(MINIFB_BUILD_EXAMPLES     OFF)
set(MINIFB_BUILD_VERSION_INFO OFF)
FetchContent_Declare(minifb
	URL      https://github.com/emoon/minifb/archive/2ca56860a2be5f62414ebe6bfaf385716d75327f.tar.gz
	URL_HASH SHA256=4268a772f7590c113e9cdf6710beff9b465723784c0bfe142be2f92b59419630
	EXCLUDE_FROM_ALL
	SYSTEM
)
FetchContent_MakeAvailable(minifb)

set(MINIAUDIO_BUILD_EXAMPLES OFF)
set(MINIAUDIO_BUILD_TESTS    OFF)
set(MINIAUDIO_BUILD_TOOLS    OFF)
FetchContent_Declare(miniaudio
	URL      https://github.com/mackron/miniaudio/archive/9634bedb5b5a2ca38c1ee7108a9358a4e233f14d.tar.gz
	URL_HASH SHA256=1a3a79b80fc6f0b0cc155e28b954a598e0ddfa2db64e2afa8466be88c476fa55
	EXCLUDE_FROM_ALL
	SYSTEM
)
FetchContent_MakeAvailable(miniaudio)

target_compile_definitions(miniaudio PUBLIC
	MA_NO_FLAC MA_NO_MP3   # WAV only
	MA_NO_ENCODING         # playback only, never write
	MA_NO_GENERATION       # no synthesized waveforms/noise
	MA_NO_STDIO            # resources are embedded
	MA_NO_ENGINE MA_NO_NODE_GRAPH MA_NO_RESOURCE_MANAGER
)
if (WIN32)
	target_compile_definitions(miniaudio PUBLIC MA_ENABLE_ONLY_SPECIFIC_BACKENDS MA_ENABLE_WASAPI)
elseif (UNIX AND NOT APPLE)
	target_compile_definitions(miniaudio PUBLIC MA_ENABLE_ONLY_SPECIFIC_BACKENDS MA_ENABLE_ALSA MA_ENABLE_PULSEAUDIO)
endif()

FetchContent_Declare(stb
	URL      https://github.com/nothings/stb/archive/2c980bb59875b0d32144a71867fbdebb2f77cd20.tar.gz
	URL_HASH SHA256=9a955b1b49a4410088a2e0ee2a9c057c3c907d0c1d75454144cb980aca0ba515
	EXCLUDE_FROM_ALL
	SYSTEM
)
FetchContent_MakeAvailable(stb)

# stb is header-only: generate one TU per header that defines its implementation macro
function(tp_impl out_var header macro)
	get_filename_component(_name "${header}" NAME_WE)
	set(_tu "${CMAKE_CURRENT_BINARY_DIR}/${_name}_impl.c")
	set(_content "#define ${macro}\n#include \"${header}\"\n")

	set(_old "")
	if (EXISTS "${_tu}")
		file(READ "${_tu}" _old)
	endif()
	if (NOT _old STREQUAL _content)
		file(WRITE "${_tu}" "${_content}")
	endif()

	set(${out_var} "${_tu}" PARENT_SCOPE)
endfunction()

add_library(stb STATIC)
add_library(tp::stb ALIAS stb)
tp_impl(TU_SI "${stb_SOURCE_DIR}/stb_image.h"    STB_IMAGE_IMPLEMENTATION)
tp_impl(TU_TT "${stb_SOURCE_DIR}/stb_truetype.h" STB_TRUETYPE_IMPLEMENTATION)
target_sources(stb PRIVATE ${TU_SI} ${TU_TT})
target_include_directories(stb SYSTEM PUBLIC "${stb_SOURCE_DIR}")
target_compile_definitions(stb
	PRIVATE STBI_ONLY_PNG
	PUBLIC  STBI_NO_STDIO)  # resources are embedded
target_compile_options(stb PRIVATE $<IF:$<BOOL:${MSVC}>,/w,-w>)
if (UNIX)
	target_link_libraries(stb PUBLIC m)
endif()
