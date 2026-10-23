cmake_minimum_required (VERSION 3.28)

project ("MinePlusPlus" LANGUAGES C CXX)
# Note: macOS is not an expected target

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(MPP_PROFILING "Build the runtime profiling tools (src/Profiling)" OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_MSVC_RUNTIME_LIBRARY "$<IF:$<CONFIG:Debug>,MultiThreadedDebugDLL,MultiThreaded>")

add_subdirectory(thirdparty) # Third party dependencies
add_subdirectory(res) # Resources embedding

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
list(FILTER SOURCES EXCLUDE REGEX "/src/Profiling/")
list(FILTER HEADERS EXCLUDE REGEX "/src/Profiling/")

add_executable(MinePlusPlus ${SOURCES} ${HEADERS})
target_include_directories(MinePlusPlus PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

set_property(TARGET MinePlusPlus PROPERTY CXX_STANDARD 20)

if (MSVC)
	target_compile_options(MinePlusPlus PRIVATE /MP /W4 /permissive-)
	target_link_options(MinePlusPlus PRIVATE "$<$<NOT:$<CONFIG:Debug>>:/NODEFAULTLIB:libucrt.lib;/DEFAULTLIB:ucrt.lib>")
else()
	target_compile_options(MinePlusPlus PRIVATE -Wall -Wextra)
endif()

target_compile_features(MinePlusPlus PRIVATE cxx_std_20)
set_target_properties(MinePlusPlus PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(MinePlusPlus PRIVATE minifb::minifb miniaudio assets)
if (WIN32)
	target_link_libraries(MinePlusPlus PRIVATE imm32)
endif()

if (MPP_PROFILING)
	if (NOT WIN32)
		message(FATAL_ERROR "MPP_PROFILING is only implemented on Windows")
	endif()
	file(GLOB_RECURSE PROFILING_FILES CONFIGURE_DEPENDS
		"${CMAKE_CURRENT_SOURCE_DIR}/src/Profiling/*.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/src/Profiling/*.h")
	target_sources(MinePlusPlus PRIVATE ${PROFILING_FILES})
	target_compile_definitions(MinePlusPlus PRIVATE MPP_PROFILING)
	target_link_libraries(MinePlusPlus PRIVATE dbghelp delayimp)
	target_link_options(MinePlusPlus PRIVATE /DELAYLOAD:dbghelp.dll)
endif()

if (UNIX AND NOT APPLE)
	find_package(Threads REQUIRED)
	target_link_libraries(MinePlusPlus PRIVATE Threads::Threads ${CMAKE_DL_LIBS})
endif()
