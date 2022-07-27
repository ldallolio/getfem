# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/ldallolio/dvp/getfem

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ldallolio/dvp/getfem/build

# Include any dependencies generated for this target.
include contrib/level_set_contact/CMakeFiles/level_set_contact.dir/depend.make

# Include the progress variables for this target.
include contrib/level_set_contact/CMakeFiles/level_set_contact.dir/progress.make

# Include the compile flags for this target's objects.
include contrib/level_set_contact/CMakeFiles/level_set_contact.dir/flags.make

contrib/level_set_contact/CMakeFiles/level_set_contact.dir/contact_problem.cpp.o: contrib/level_set_contact/CMakeFiles/level_set_contact.dir/flags.make
contrib/level_set_contact/CMakeFiles/level_set_contact.dir/contact_problem.cpp.o: ../contrib/level_set_contact/contact_problem.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ldallolio/dvp/getfem/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object contrib/level_set_contact/CMakeFiles/level_set_contact.dir/contact_problem.cpp.o"
	cd /home/ldallolio/dvp/getfem/build/contrib/level_set_contact && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/level_set_contact.dir/contact_problem.cpp.o -c /home/ldallolio/dvp/getfem/contrib/level_set_contact/contact_problem.cpp

contrib/level_set_contact/CMakeFiles/level_set_contact.dir/contact_problem.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/level_set_contact.dir/contact_problem.cpp.i"
	cd /home/ldallolio/dvp/getfem/build/contrib/level_set_contact && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ldallolio/dvp/getfem/contrib/level_set_contact/contact_problem.cpp > CMakeFiles/level_set_contact.dir/contact_problem.cpp.i

contrib/level_set_contact/CMakeFiles/level_set_contact.dir/contact_problem.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/level_set_contact.dir/contact_problem.cpp.s"
	cd /home/ldallolio/dvp/getfem/build/contrib/level_set_contact && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ldallolio/dvp/getfem/contrib/level_set_contact/contact_problem.cpp -o CMakeFiles/level_set_contact.dir/contact_problem.cpp.s

contrib/level_set_contact/CMakeFiles/level_set_contact.dir/test_contact.cpp.o: contrib/level_set_contact/CMakeFiles/level_set_contact.dir/flags.make
contrib/level_set_contact/CMakeFiles/level_set_contact.dir/test_contact.cpp.o: ../contrib/level_set_contact/test_contact.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ldallolio/dvp/getfem/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object contrib/level_set_contact/CMakeFiles/level_set_contact.dir/test_contact.cpp.o"
	cd /home/ldallolio/dvp/getfem/build/contrib/level_set_contact && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/level_set_contact.dir/test_contact.cpp.o -c /home/ldallolio/dvp/getfem/contrib/level_set_contact/test_contact.cpp

contrib/level_set_contact/CMakeFiles/level_set_contact.dir/test_contact.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/level_set_contact.dir/test_contact.cpp.i"
	cd /home/ldallolio/dvp/getfem/build/contrib/level_set_contact && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ldallolio/dvp/getfem/contrib/level_set_contact/test_contact.cpp > CMakeFiles/level_set_contact.dir/test_contact.cpp.i

contrib/level_set_contact/CMakeFiles/level_set_contact.dir/test_contact.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/level_set_contact.dir/test_contact.cpp.s"
	cd /home/ldallolio/dvp/getfem/build/contrib/level_set_contact && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ldallolio/dvp/getfem/contrib/level_set_contact/test_contact.cpp -o CMakeFiles/level_set_contact.dir/test_contact.cpp.s

# Object files for target level_set_contact
level_set_contact_OBJECTS = \
"CMakeFiles/level_set_contact.dir/contact_problem.cpp.o" \
"CMakeFiles/level_set_contact.dir/test_contact.cpp.o"

# External object files for target level_set_contact
level_set_contact_EXTERNAL_OBJECTS =

contrib/level_set_contact/liblevel_set_contact.a: contrib/level_set_contact/CMakeFiles/level_set_contact.dir/contact_problem.cpp.o
contrib/level_set_contact/liblevel_set_contact.a: contrib/level_set_contact/CMakeFiles/level_set_contact.dir/test_contact.cpp.o
contrib/level_set_contact/liblevel_set_contact.a: contrib/level_set_contact/CMakeFiles/level_set_contact.dir/build.make
contrib/level_set_contact/liblevel_set_contact.a: contrib/level_set_contact/CMakeFiles/level_set_contact.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ldallolio/dvp/getfem/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX static library liblevel_set_contact.a"
	cd /home/ldallolio/dvp/getfem/build/contrib/level_set_contact && $(CMAKE_COMMAND) -P CMakeFiles/level_set_contact.dir/cmake_clean_target.cmake
	cd /home/ldallolio/dvp/getfem/build/contrib/level_set_contact && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/level_set_contact.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
contrib/level_set_contact/CMakeFiles/level_set_contact.dir/build: contrib/level_set_contact/liblevel_set_contact.a

.PHONY : contrib/level_set_contact/CMakeFiles/level_set_contact.dir/build

contrib/level_set_contact/CMakeFiles/level_set_contact.dir/clean:
	cd /home/ldallolio/dvp/getfem/build/contrib/level_set_contact && $(CMAKE_COMMAND) -P CMakeFiles/level_set_contact.dir/cmake_clean.cmake
.PHONY : contrib/level_set_contact/CMakeFiles/level_set_contact.dir/clean

contrib/level_set_contact/CMakeFiles/level_set_contact.dir/depend:
	cd /home/ldallolio/dvp/getfem/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ldallolio/dvp/getfem /home/ldallolio/dvp/getfem/contrib/level_set_contact /home/ldallolio/dvp/getfem/build /home/ldallolio/dvp/getfem/build/contrib/level_set_contact /home/ldallolio/dvp/getfem/build/contrib/level_set_contact/CMakeFiles/level_set_contact.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : contrib/level_set_contact/CMakeFiles/level_set_contact.dir/depend

