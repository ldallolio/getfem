# Generating the library static_contact_gears
set(project "static_contact_gears")

set(${project}, "")
list(APPEND ${project}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/static_contact_gears.cc
)
add_executable(static_contact_gears ${${project}_SOURCES} )
# Generating the library static_contact_gears_u1_u2
set(project "static_contact_gears_u1_u2")

set(${project}, "")
list(APPEND ${project}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/static_contact_gears_u1_u2.cc
)
add_executable(static_contact_gears_u1_u2 ${${project}_SOURCES} )
