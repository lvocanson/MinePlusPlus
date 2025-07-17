cmake_minimum_required (VERSION 3.12)
project ("MinePlusPlus")

add_executable (MinePlusPlus "src/MinePlusPlus.cpp" )

set_property(TARGET MinePlusPlus PROPERTY CXX_STANDARD 20)
