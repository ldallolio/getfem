cmake_minimum_required(VERSION 2.8...3.20)
set (project delaminated_crack)
set(${project}_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/delaminated_crack.cc
)
add_library(${project} STATIC ${${project}_SOURCES} )