cmake_minimum_required(VERSION 2.8)
set (project bimaterial_crack_test)
set(${project}_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/bimaterial_crack_test.cc
	${CMAKE_CURRENT_SOURCE_DIR}/getfem_Xfem.cc
	${CMAKE_CURRENT_SOURCE_DIR}/crack_exact_solution.cc
	${CMAKE_CURRENT_SOURCE_DIR}/crack.cc
	${CMAKE_CURRENT_SOURCE_DIR}/getfem_spider_fem.h
	${CMAKE_CURRENT_SOURCE_DIR}/crack_exact_solution.h
	${CMAKE_CURRENT_SOURCE_DIR}/getfem_Xfem.h
)
add_library(${project} STATIC ${${project}_SOURCES} )