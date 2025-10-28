cmake_minimum_required (VERSION 3.12)
project ("MinePlusPlus")

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

include(FetchContent)
FetchContent_Declare(SFML
	GIT_REPOSITORY https://github.com/SFML/SFML.git
	GIT_TAG 3.0.1
	GIT_SHALLOW ON
	EXCLUDE_FROM_ALL
	SYSTEM
)
FetchContent_MakeAvailable(SFML)

add_executable(MinePlusPlus "src/MinePlusPlus.cpp" "src/Board.h" "src/Board.cpp" "src/Resources.h" "src/Layer/MinesweeperLayer.h" "src/Layer/MinesweeperLayer.cpp" "src/SFMLExtensions.h" "src/SFMLExtensions.cpp" "src/MyRandom.h" "src/App.h" "src/App.cpp" "src/Layer/MenuLayer.h" "src/Layer/MenuLayer.cpp" "src/Layer/Layer.h" "src/Layer/GameUILayer.h" "src/Layer/GameUILayer.cpp" "src/Event/EventBus.h" "src/Event/EventBus.cpp" "src/Event/MinesweeperEvents.h" "src/Layer/BaseUILayer.h" "src/Layer/BaseUILayer.cpp")
target_include_directories(MinePlusPlus PRIVATE "${CMAKE_SOURCE_DIR}/src")

set_property(TARGET MinePlusPlus PROPERTY CXX_STANDARD 20)

# Multi-processor compilation
if (MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE /MP)
endif()

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_link_libraries(${PROJECT_NAME} PRIVATE SFML::Graphics)

add_custom_command(
	TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_BINARY_DIR}/res"
	COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/res" "${CMAKE_BINARY_DIR}/res"
	COMMAND ${CMAKE_COMMAND} -E echo "Replaced the 'res' folder in the output directory."
)
