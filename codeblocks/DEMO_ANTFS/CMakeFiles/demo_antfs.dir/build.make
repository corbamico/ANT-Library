# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

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

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = /usr/bin/ccmake

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/picuntu/github/ANT-Library/src

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/picuntu/github/ANT-Library/codeblocks

# Include any dependencies generated for this target.
include DEMO_ANTFS/CMakeFiles/demo_antfs.dir/depend.make

# Include the progress variables for this target.
include DEMO_ANTFS/CMakeFiles/demo_antfs.dir/progress.make

# Include the compile flags for this target's objects.
include DEMO_ANTFS/CMakeFiles/demo_antfs.dir/flags.make

DEMO_ANTFS/CMakeFiles/demo_antfs.dir/demo_antfs.cpp.o: DEMO_ANTFS/CMakeFiles/demo_antfs.dir/flags.make
DEMO_ANTFS/CMakeFiles/demo_antfs.dir/demo_antfs.cpp.o: /home/picuntu/github/ANT-Library/src/DEMO_ANTFS/demo_antfs.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/picuntu/github/ANT-Library/codeblocks/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object DEMO_ANTFS/CMakeFiles/demo_antfs.dir/demo_antfs.cpp.o"
	cd /home/picuntu/github/ANT-Library/codeblocks/DEMO_ANTFS && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/demo_antfs.dir/demo_antfs.cpp.o -c /home/picuntu/github/ANT-Library/src/DEMO_ANTFS/demo_antfs.cpp

DEMO_ANTFS/CMakeFiles/demo_antfs.dir/demo_antfs.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/demo_antfs.dir/demo_antfs.cpp.i"
	cd /home/picuntu/github/ANT-Library/codeblocks/DEMO_ANTFS && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/picuntu/github/ANT-Library/src/DEMO_ANTFS/demo_antfs.cpp > CMakeFiles/demo_antfs.dir/demo_antfs.cpp.i

DEMO_ANTFS/CMakeFiles/demo_antfs.dir/demo_antfs.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/demo_antfs.dir/demo_antfs.cpp.s"
	cd /home/picuntu/github/ANT-Library/codeblocks/DEMO_ANTFS && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/picuntu/github/ANT-Library/src/DEMO_ANTFS/demo_antfs.cpp -o CMakeFiles/demo_antfs.dir/demo_antfs.cpp.s

DEMO_ANTFS/CMakeFiles/demo_antfs.dir/demo_antfs.cpp.o.requires:
.PHONY : DEMO_ANTFS/CMakeFiles/demo_antfs.dir/demo_antfs.cpp.o.requires

DEMO_ANTFS/CMakeFiles/demo_antfs.dir/demo_antfs.cpp.o.provides: DEMO_ANTFS/CMakeFiles/demo_antfs.dir/demo_antfs.cpp.o.requires
	$(MAKE) -f DEMO_ANTFS/CMakeFiles/demo_antfs.dir/build.make DEMO_ANTFS/CMakeFiles/demo_antfs.dir/demo_antfs.cpp.o.provides.build
.PHONY : DEMO_ANTFS/CMakeFiles/demo_antfs.dir/demo_antfs.cpp.o.provides

DEMO_ANTFS/CMakeFiles/demo_antfs.dir/demo_antfs.cpp.o.provides.build: DEMO_ANTFS/CMakeFiles/demo_antfs.dir/demo_antfs.cpp.o

# Object files for target demo_antfs
demo_antfs_OBJECTS = \
"CMakeFiles/demo_antfs.dir/demo_antfs.cpp.o"

# External object files for target demo_antfs
demo_antfs_EXTERNAL_OBJECTS =

DEMO_ANTFS/demo_antfs: DEMO_ANTFS/CMakeFiles/demo_antfs.dir/demo_antfs.cpp.o
DEMO_ANTFS/demo_antfs: DEMO_ANTFS/CMakeFiles/demo_antfs.dir/build.make
DEMO_ANTFS/demo_antfs: ANT_LIB/libant.a
DEMO_ANTFS/demo_antfs: DEMO_ANTFS/CMakeFiles/demo_antfs.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable demo_antfs"
	cd /home/picuntu/github/ANT-Library/codeblocks/DEMO_ANTFS && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/demo_antfs.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
DEMO_ANTFS/CMakeFiles/demo_antfs.dir/build: DEMO_ANTFS/demo_antfs
.PHONY : DEMO_ANTFS/CMakeFiles/demo_antfs.dir/build

DEMO_ANTFS/CMakeFiles/demo_antfs.dir/requires: DEMO_ANTFS/CMakeFiles/demo_antfs.dir/demo_antfs.cpp.o.requires
.PHONY : DEMO_ANTFS/CMakeFiles/demo_antfs.dir/requires

DEMO_ANTFS/CMakeFiles/demo_antfs.dir/clean:
	cd /home/picuntu/github/ANT-Library/codeblocks/DEMO_ANTFS && $(CMAKE_COMMAND) -P CMakeFiles/demo_antfs.dir/cmake_clean.cmake
.PHONY : DEMO_ANTFS/CMakeFiles/demo_antfs.dir/clean

DEMO_ANTFS/CMakeFiles/demo_antfs.dir/depend:
	cd /home/picuntu/github/ANT-Library/codeblocks && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/picuntu/github/ANT-Library/src /home/picuntu/github/ANT-Library/src/DEMO_ANTFS /home/picuntu/github/ANT-Library/codeblocks /home/picuntu/github/ANT-Library/codeblocks/DEMO_ANTFS /home/picuntu/github/ANT-Library/codeblocks/DEMO_ANTFS/CMakeFiles/demo_antfs.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : DEMO_ANTFS/CMakeFiles/demo_antfs.dir/depend
