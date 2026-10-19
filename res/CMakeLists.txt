# Embeds each resource file as a byte array compiled once into the `assets` target,
# exposed as a view:  namespace assets { std::span<const unsigned char> <name>(); }
function(embed_asset out_src name file)
	if (NOT EXISTS "${file}")
		message(FATAL_ERROR "embed_asset: missing file: ${file}")
	endif()

	set(_dir "${CMAKE_CURRENT_BINARY_DIR}/assets")
	set(_hdr "${_dir}/${name}.h")
	set(_src "${_dir}/${name}.cpp")

	file(READ "${file}" _hex HEX)
	string(LENGTH "${_hex}" _hexlen)
	math(EXPR _count "${_hexlen} / 2")
	string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," _init "${_hex}")

	set(_hdr_content "
#pragma once
#include <span>
namespace assets { std::span<const unsigned char> ${name}() noexcept; }
")
	set(_src_content "
#include \"${name}.h\"
namespace { const unsigned char ${name}_data[${_count}] = { ${_init} }; }
namespace assets { std::span<const unsigned char> ${name}() noexcept { return { ${name}_data, ${_count} }; } }
")

	set(_old "")
	if (EXISTS "${_hdr}")
		file(READ "${_hdr}" _old)
	endif()
	if (NOT _old STREQUAL _hdr_content)
		file(WRITE "${_hdr}" "${_hdr_content}")
	endif()

	set(_old "")
	if (EXISTS "${_src}")
		file(READ "${_src}" _old)
	endif()
	if (NOT _old STREQUAL _src_content)
		file(WRITE "${_src}" "${_src_content}")
	endif()

	set(${out_src} "${_src}" PARENT_SCOPE)
endfunction()

add_library(assets STATIC)
add_library(mpp::assets ALIAS assets)

embed_asset(A_ATLAS cells_atlas_png "${CMAKE_CURRENT_SOURCE_DIR}/cells-atlas.png")
embed_asset(A_FONT  roboto_regular  "${CMAKE_CURRENT_SOURCE_DIR}/roboto-v48-latin-regular.ttf")
embed_asset(A_CLK1  click_1         "${CMAKE_CURRENT_SOURCE_DIR}/click-1.wav")
embed_asset(A_CLK2  click_2         "${CMAKE_CURRENT_SOURCE_DIR}/click-2.wav")
embed_asset(A_EXPL  explosion       "${CMAKE_CURRENT_SOURCE_DIR}/explosion.wav")
embed_asset(A_VICT  victory         "${CMAKE_CURRENT_SOURCE_DIR}/victory.wav")

target_sources(assets PRIVATE ${A_ATLAS} ${A_FONT} ${A_CLK1} ${A_CLK2} ${A_EXPL} ${A_VICT})
target_include_directories(assets PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/assets")
target_compile_features(assets PUBLIC cxx_std_20)
set_target_properties(assets PROPERTIES CXX_EXTENSIONS OFF)
