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
CMAKE_SOURCE_DIR = /home/libo/Modern_Cplusplus/concurrency_book_code/initial_lock

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/libo/Modern_Cplusplus/concurrency_book_code/initial_lock/build

# Include any dependencies generated for this target.
include CMakeFiles/initial_lock.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/initial_lock.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/initial_lock.dir/flags.make

CMakeFiles/initial_lock.dir/initial_lock.cpp.o: CMakeFiles/initial_lock.dir/flags.make
CMakeFiles/initial_lock.dir/initial_lock.cpp.o: ../initial_lock.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/libo/Modern_Cplusplus/concurrency_book_code/initial_lock/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/initial_lock.dir/initial_lock.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/initial_lock.dir/initial_lock.cpp.o -c /home/libo/Modern_Cplusplus/concurrency_book_code/initial_lock/initial_lock.cpp

CMakeFiles/initial_lock.dir/initial_lock.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/initial_lock.dir/initial_lock.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/libo/Modern_Cplusplus/concurrency_book_code/initial_lock/initial_lock.cpp > CMakeFiles/initial_lock.dir/initial_lock.cpp.i

CMakeFiles/initial_lock.dir/initial_lock.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/initial_lock.dir/initial_lock.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/libo/Modern_Cplusplus/concurrency_book_code/initial_lock/initial_lock.cpp -o CMakeFiles/initial_lock.dir/initial_lock.cpp.s

# Object files for target initial_lock
initial_lock_OBJECTS = \
"CMakeFiles/initial_lock.dir/initial_lock.cpp.o"

# External object files for target initial_lock
initial_lock_EXTERNAL_OBJECTS =

initial_lock: CMakeFiles/initial_lock.dir/initial_lock.cpp.o
initial_lock: CMakeFiles/initial_lock.dir/build.make
initial_lock: CMakeFiles/initial_lock.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/libo/Modern_Cplusplus/concurrency_book_code/initial_lock/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable initial_lock"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/initial_lock.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/initial_lock.dir/build: initial_lock

.PHONY : CMakeFiles/initial_lock.dir/build

CMakeFiles/initial_lock.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/initial_lock.dir/cmake_clean.cmake
.PHONY : CMakeFiles/initial_lock.dir/clean

CMakeFiles/initial_lock.dir/depend:
	cd /home/libo/Modern_Cplusplus/concurrency_book_code/initial_lock/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/libo/Modern_Cplusplus/concurrency_book_code/initial_lock /home/libo/Modern_Cplusplus/concurrency_book_code/initial_lock /home/libo/Modern_Cplusplus/concurrency_book_code/initial_lock/build /home/libo/Modern_Cplusplus/concurrency_book_code/initial_lock/build /home/libo/Modern_Cplusplus/concurrency_book_code/initial_lock/build/CMakeFiles/initial_lock.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/initial_lock.dir/depend

