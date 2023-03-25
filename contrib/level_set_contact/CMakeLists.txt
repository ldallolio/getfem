cmake_minimum_required(VERSION 2.8...3.20)
set (project level_set_contact)

set(${project}_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/contact_problem.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/test_contact.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/contact_problem.h
)
add_library(${project} STATIC ${${project}_SOURCES} )