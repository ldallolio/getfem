# Generating the library aposteriori
set(project "aposteriori")

set(${project}, "")
list(APPEND ${project}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/aposteriori.cc
)
add_executable(aposteriori ${${project}_SOURCES} )
# Generating the library aposteriori_laplacian
set(project "aposteriori_laplacian")

set(${project}, "")
list(APPEND ${project}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/aposteriori_laplacian.cc
)
add_executable(aposteriori_laplacian ${${project}_SOURCES} )
