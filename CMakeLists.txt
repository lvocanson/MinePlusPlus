cmake_minimum_required (VERSION 3.28)

project ("MinePlusPlus" LANGUAGES C CXX)
# Note: macOS is not an expected target

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

add_subdirectory(thirdparty) # Third party dependencies
add_subdirectory(res) # Resources embedding

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")

add_executable(MinePlusPlus ${SOURCES} ${HEADERS})
target_include_directories(MinePlusPlus PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

set_property(TARGET MinePlusPlus PROPERTY CXX_STANDARD 20)

if (MSVC)
	target_compile_options(MinePlusPlus PRIVATE /MP /W4 /permissive-)
else()
	target_compile_options(MinePlusPlus PRIVATE -Wall -Wextra)
endif()

target_compile_features(MinePlusPlus PRIVATE cxx_std_20)
set_target_properties(MinePlusPlus PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(MinePlusPlus PRIVATE minifb::minifb tp::stb miniaudio assets)

if (UNIX AND NOT APPLE)
	find_package(Threads REQUIRED)
	target_link_libraries(MinePlusPlus PRIVATE Threads::Threads ${CMAKE_DL_LIBS})
endif()
