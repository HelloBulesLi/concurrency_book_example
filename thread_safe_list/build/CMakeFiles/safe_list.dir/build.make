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
CMAKE_SOURCE_DIR = /home/libo/Modern_Cplusplus/concurrency_book_code/thread_safe_list

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/libo/Modern_Cplusplus/concurrency_book_code/thread_safe_list/build

# Include any dependencies generated for this target.
include CMakeFiles/safe_list.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/safe_list.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/safe_list.dir/flags.make

CMakeFiles/safe_list.dir/safe_list.cpp.o: CMakeFiles/safe_list.dir/flags.make
CMakeFiles/safe_list.dir/safe_list.cpp.o: ../safe_list.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/libo/Modern_Cplusplus/concurrency_book_code/thread_safe_list/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/safe_list.dir/safe_list.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/safe_list.dir/safe_list.cpp.o -c /home/libo/Modern_Cplusplus/concurrency_book_code/thread_safe_list/safe_list.cpp

CMakeFiles/safe_list.dir/safe_list.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/safe_list.dir/safe_list.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/libo/Modern_Cplusplus/concurrency_book_code/thread_safe_list/safe_list.cpp > CMakeFiles/safe_list.dir/safe_list.cpp.i

CMakeFiles/safe_list.dir/safe_list.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/safe_list.dir/safe_list.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/libo/Modern_Cplusplus/concurrency_book_code/thread_safe_list/safe_list.cpp -o CMakeFiles/safe_list.dir/safe_list.cpp.s

# Object files for target safe_list
safe_list_OBJECTS = \
"CMakeFiles/safe_list.dir/safe_list.cpp.o"

# External object files for target safe_list
safe_list_EXTERNAL_OBJECTS =

safe_list: CMakeFiles/safe_list.dir/safe_list.cpp.o
safe_list: CMakeFiles/safe_list.dir/build.make
safe_list: CMakeFiles/safe_list.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/libo/Modern_Cplusplus/concurrency_book_code/thread_safe_list/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable safe_list"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/safe_list.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/safe_list.dir/build: safe_list

.PHONY : CMakeFiles/safe_list.dir/build

CMakeFiles/safe_list.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/safe_list.dir/cmake_clean.cmake
.PHONY : CMakeFiles/safe_list.dir/clean

CMakeFiles/safe_list.dir/depend:
	cd /home/libo/Modern_Cplusplus/concurrency_book_code/thread_safe_list/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/libo/Modern_Cplusplus/concurrency_book_code/thread_safe_list /home/libo/Modern_Cplusplus/concurrency_book_code/thread_safe_list /home/libo/Modern_Cplusplus/concurrency_book_code/thread_safe_list/build /home/libo/Modern_Cplusplus/concurrency_book_code/thread_safe_list/build /home/libo/Modern_Cplusplus/concurrency_book_code/thread_safe_list/build/CMakeFiles/safe_list.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/safe_list.dir/depend

