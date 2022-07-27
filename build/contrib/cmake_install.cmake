# Install script for directory: /home/ldallolio/dvp/getfem/contrib

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/ldallolio/dvp/getfem/build/contrib/icare/cmake_install.cmake")
  include("/home/ldallolio/dvp/getfem/build/contrib/delaminated_crack/cmake_install.cmake")
  include("/home/ldallolio/dvp/getfem/build/contrib/aposteriori/cmake_install.cmake")
  include("/home/ldallolio/dvp/getfem/build/contrib/xfem_stab_unilat_contact/cmake_install.cmake")
  include("/home/ldallolio/dvp/getfem/build/contrib/bimaterial_crack_test/cmake_install.cmake")
  include("/home/ldallolio/dvp/getfem/build/contrib/mixed_elastostatic/cmake_install.cmake")
  include("/home/ldallolio/dvp/getfem/build/contrib/xfem_contact/cmake_install.cmake")
  include("/home/ldallolio/dvp/getfem/build/contrib/crack_plate/cmake_install.cmake")
  include("/home/ldallolio/dvp/getfem/build/contrib/static_contact_gears/cmake_install.cmake")
  include("/home/ldallolio/dvp/getfem/build/contrib/level_set_contact/cmake_install.cmake")
  include("/home/ldallolio/dvp/getfem/build/contrib/opt_assembly/cmake_install.cmake")

endif()

