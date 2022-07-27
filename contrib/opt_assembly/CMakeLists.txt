# Generating the library opt_assembly
set(project "opt_assembly")

set(${project}, "")
list(APPEND ${project}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/opt_assembly.cc
)
add_executable(opt_assembly ${${project}_SOURCES} )
