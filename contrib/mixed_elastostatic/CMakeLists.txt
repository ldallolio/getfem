cmake_minimum_required(VERSION 2.8)
set (project mixed_elastostatic)
set(${project}_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/mixed_elastostatic.cc
)
add_library(${project} STATIC ${${project}_SOURCES} )