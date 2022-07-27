# Generating the library xfem_contact
set(project "xfem_contact")

list(APPEND ${project}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/xfem_contact.cc
)
add_executable(xfem_contact ${${project}_SOURCES} )
# Generating the library xfem_stokes
set(project "xfem_stokes")

list(APPEND ${project}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/xfem_stokes.cc
)
add_executable(xfem_stokes ${${project}_SOURCES} )
# Generating the library optprogs
set(project "xfem_dirichlet")

list(APPEND ${project}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/xfem_dirichlet.cc
)
add_executable(xfem_dirichlet ${${project}_SOURCES} )
