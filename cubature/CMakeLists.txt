cmake_minimum_required(VERSION 2.8)
set(project cubature)

## IM_METHODS
#execute_process(COMMAND
#	${PROJECT_SOURCE_DIR}/bin/extract_doc
#	${PROJECT_SOURCE_DIR}/interface/src cubature
#	${CMAKE_BINARY_DIR})

## IM_METHODS_LOC
#execute_process(COMMAND
#	${PROJECT_SOURCE_DIR}/bin/extract_doc
#	${PROJECT_SOURCE_DIR}/interface/src cubature_loc
#	${CMAKE_BINARY_DIR})	

#execute_process(COMMAND
#	${PERL_EXECUTABLE}
#	${PROJECT_SOURCE_DIR}/cubature/make_getfem_im_list
#	getfem_im_list.h
#	${CMAKE_BINARY_DIR})

# add_library(${project} STATIC ${${project}_SOURCES} )