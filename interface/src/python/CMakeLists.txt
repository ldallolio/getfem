cmake_minimum_required(VERSION 2.8...3.20)
set (project python)

set(${project}_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/getfem_python.c
)
add_library(${project} STATIC ${${project}_SOURCES} )