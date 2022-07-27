cmake_minimum_required(VERSION 2.8)
set (project icare)
set(${project}_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/icare.cc
	${CMAKE_CURRENT_SOURCE_DIR}/icare.h
)
add_library(${project} STATIC ${${project}_SOURCES} )