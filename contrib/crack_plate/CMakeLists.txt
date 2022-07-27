# Generating the library crack_mindlin
set(project "crack_mindlin")

set(${project}, "")
if (QHULL)
    list(APPEND ${project}_SOURCES
    
    ${CMAKE_CURRENT_SOURCE_DIR}/crack_mindlin.cc)
endif()

if (QHULL)
add_executable(crack_mindlin ${${project}_SOURCES} )

endif()
# Generating the library crack_bilaplacian
set(project "crack_bilaplacian")

set(${project}, "")
if (QHULL)
    list(APPEND ${project}_SOURCES
    
    ${CMAKE_CURRENT_SOURCE_DIR}/crack_bilaplacian.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/crack_bilaplacian.h
    ${CMAKE_CURRENT_SOURCE_DIR}/crack_bilaplacian_moment.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/crack_bilaplacian_problem.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/crack_bilaplacian_sif.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/crack_bilaplacian_singularities.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/crack_bilaplacian_tools.cc)
endif()

if (QHULL)
add_executable(crack_bilaplacian ${${project}_SOURCES} )

endif()
