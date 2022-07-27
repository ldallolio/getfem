# Generating the library xfem_stab_unilat_contact
set(project "xfem_stab_unilat_contact")

set(${project}, "")
list(APPEND ${project}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/xfem_stab_unilat_contact.cc
)
add_executable(xfem_stab_unilat_contact ${${project}_SOURCES} )
