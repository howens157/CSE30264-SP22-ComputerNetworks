# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.22.3/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.22.3/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c-build"

# Include any dependencies generated for this target.
include CMakeFiles/json-c-static.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/json-c-static.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/json-c-static.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/json-c-static.dir/flags.make

CMakeFiles/json-c-static.dir/arraylist.c.o: CMakeFiles/json-c-static.dir/flags.make
CMakeFiles/json-c-static.dir/arraylist.c.o: /Users/rgriffith/Documents/Computer\ Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/arraylist.c
CMakeFiles/json-c-static.dir/arraylist.c.o: CMakeFiles/json-c-static.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c-build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/json-c-static.dir/arraylist.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/json-c-static.dir/arraylist.c.o -MF CMakeFiles/json-c-static.dir/arraylist.c.o.d -o CMakeFiles/json-c-static.dir/arraylist.c.o -c "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/arraylist.c"

CMakeFiles/json-c-static.dir/arraylist.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/json-c-static.dir/arraylist.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/arraylist.c" > CMakeFiles/json-c-static.dir/arraylist.c.i

CMakeFiles/json-c-static.dir/arraylist.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/json-c-static.dir/arraylist.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/arraylist.c" -o CMakeFiles/json-c-static.dir/arraylist.c.s

CMakeFiles/json-c-static.dir/debug.c.o: CMakeFiles/json-c-static.dir/flags.make
CMakeFiles/json-c-static.dir/debug.c.o: /Users/rgriffith/Documents/Computer\ Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/debug.c
CMakeFiles/json-c-static.dir/debug.c.o: CMakeFiles/json-c-static.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c-build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/json-c-static.dir/debug.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/json-c-static.dir/debug.c.o -MF CMakeFiles/json-c-static.dir/debug.c.o.d -o CMakeFiles/json-c-static.dir/debug.c.o -c "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/debug.c"

CMakeFiles/json-c-static.dir/debug.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/json-c-static.dir/debug.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/debug.c" > CMakeFiles/json-c-static.dir/debug.c.i

CMakeFiles/json-c-static.dir/debug.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/json-c-static.dir/debug.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/debug.c" -o CMakeFiles/json-c-static.dir/debug.c.s

CMakeFiles/json-c-static.dir/json_c_version.c.o: CMakeFiles/json-c-static.dir/flags.make
CMakeFiles/json-c-static.dir/json_c_version.c.o: /Users/rgriffith/Documents/Computer\ Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/json_c_version.c
CMakeFiles/json-c-static.dir/json_c_version.c.o: CMakeFiles/json-c-static.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c-build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_3) "Building C object CMakeFiles/json-c-static.dir/json_c_version.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/json-c-static.dir/json_c_version.c.o -MF CMakeFiles/json-c-static.dir/json_c_version.c.o.d -o CMakeFiles/json-c-static.dir/json_c_version.c.o -c "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/json_c_version.c"

CMakeFiles/json-c-static.dir/json_c_version.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/json-c-static.dir/json_c_version.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/json_c_version.c" > CMakeFiles/json-c-static.dir/json_c_version.c.i

CMakeFiles/json-c-static.dir/json_c_version.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/json-c-static.dir/json_c_version.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/json_c_version.c" -o CMakeFiles/json-c-static.dir/json_c_version.c.s

CMakeFiles/json-c-static.dir/json_object.c.o: CMakeFiles/json-c-static.dir/flags.make
CMakeFiles/json-c-static.dir/json_object.c.o: /Users/rgriffith/Documents/Computer\ Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/json_object.c
CMakeFiles/json-c-static.dir/json_object.c.o: CMakeFiles/json-c-static.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c-build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_4) "Building C object CMakeFiles/json-c-static.dir/json_object.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/json-c-static.dir/json_object.c.o -MF CMakeFiles/json-c-static.dir/json_object.c.o.d -o CMakeFiles/json-c-static.dir/json_object.c.o -c "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/json_object.c"

CMakeFiles/json-c-static.dir/json_object.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/json-c-static.dir/json_object.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/json_object.c" > CMakeFiles/json-c-static.dir/json_object.c.i

CMakeFiles/json-c-static.dir/json_object.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/json-c-static.dir/json_object.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/json_object.c" -o CMakeFiles/json-c-static.dir/json_object.c.s

CMakeFiles/json-c-static.dir/json_object_iterator.c.o: CMakeFiles/json-c-static.dir/flags.make
CMakeFiles/json-c-static.dir/json_object_iterator.c.o: /Users/rgriffith/Documents/Computer\ Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/json_object_iterator.c
CMakeFiles/json-c-static.dir/json_object_iterator.c.o: CMakeFiles/json-c-static.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c-build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_5) "Building C object CMakeFiles/json-c-static.dir/json_object_iterator.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/json-c-static.dir/json_object_iterator.c.o -MF CMakeFiles/json-c-static.dir/json_object_iterator.c.o.d -o CMakeFiles/json-c-static.dir/json_object_iterator.c.o -c "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/json_object_iterator.c"

CMakeFiles/json-c-static.dir/json_object_iterator.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/json-c-static.dir/json_object_iterator.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/json_object_iterator.c" > CMakeFiles/json-c-static.dir/json_object_iterator.c.i

CMakeFiles/json-c-static.dir/json_object_iterator.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/json-c-static.dir/json_object_iterator.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/json_object_iterator.c" -o CMakeFiles/json-c-static.dir/json_object_iterator.c.s

CMakeFiles/json-c-static.dir/json_tokener.c.o: CMakeFiles/json-c-static.dir/flags.make
CMakeFiles/json-c-static.dir/json_tokener.c.o: /Users/rgriffith/Documents/Computer\ Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/json_tokener.c
CMakeFiles/json-c-static.dir/json_tokener.c.o: CMakeFiles/json-c-static.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c-build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_6) "Building C object CMakeFiles/json-c-static.dir/json_tokener.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/json-c-static.dir/json_tokener.c.o -MF CMakeFiles/json-c-static.dir/json_tokener.c.o.d -o CMakeFiles/json-c-static.dir/json_tokener.c.o -c "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/json_tokener.c"

CMakeFiles/json-c-static.dir/json_tokener.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/json-c-static.dir/json_tokener.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/json_tokener.c" > CMakeFiles/json-c-static.dir/json_tokener.c.i

CMakeFiles/json-c-static.dir/json_tokener.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/json-c-static.dir/json_tokener.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/json_tokener.c" -o CMakeFiles/json-c-static.dir/json_tokener.c.s

CMakeFiles/json-c-static.dir/json_util.c.o: CMakeFiles/json-c-static.dir/flags.make
CMakeFiles/json-c-static.dir/json_util.c.o: /Users/rgriffith/Documents/Computer\ Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/json_util.c
CMakeFiles/json-c-static.dir/json_util.c.o: CMakeFiles/json-c-static.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c-build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_7) "Building C object CMakeFiles/json-c-static.dir/json_util.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/json-c-static.dir/json_util.c.o -MF CMakeFiles/json-c-static.dir/json_util.c.o.d -o CMakeFiles/json-c-static.dir/json_util.c.o -c "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/json_util.c"

CMakeFiles/json-c-static.dir/json_util.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/json-c-static.dir/json_util.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/json_util.c" > CMakeFiles/json-c-static.dir/json_util.c.i

CMakeFiles/json-c-static.dir/json_util.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/json-c-static.dir/json_util.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/json_util.c" -o CMakeFiles/json-c-static.dir/json_util.c.s

CMakeFiles/json-c-static.dir/json_visit.c.o: CMakeFiles/json-c-static.dir/flags.make
CMakeFiles/json-c-static.dir/json_visit.c.o: /Users/rgriffith/Documents/Computer\ Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/json_visit.c
CMakeFiles/json-c-static.dir/json_visit.c.o: CMakeFiles/json-c-static.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c-build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_8) "Building C object CMakeFiles/json-c-static.dir/json_visit.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/json-c-static.dir/json_visit.c.o -MF CMakeFiles/json-c-static.dir/json_visit.c.o.d -o CMakeFiles/json-c-static.dir/json_visit.c.o -c "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/json_visit.c"

CMakeFiles/json-c-static.dir/json_visit.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/json-c-static.dir/json_visit.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/json_visit.c" > CMakeFiles/json-c-static.dir/json_visit.c.i

CMakeFiles/json-c-static.dir/json_visit.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/json-c-static.dir/json_visit.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/json_visit.c" -o CMakeFiles/json-c-static.dir/json_visit.c.s

CMakeFiles/json-c-static.dir/linkhash.c.o: CMakeFiles/json-c-static.dir/flags.make
CMakeFiles/json-c-static.dir/linkhash.c.o: /Users/rgriffith/Documents/Computer\ Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/linkhash.c
CMakeFiles/json-c-static.dir/linkhash.c.o: CMakeFiles/json-c-static.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c-build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_9) "Building C object CMakeFiles/json-c-static.dir/linkhash.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/json-c-static.dir/linkhash.c.o -MF CMakeFiles/json-c-static.dir/linkhash.c.o.d -o CMakeFiles/json-c-static.dir/linkhash.c.o -c "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/linkhash.c"

CMakeFiles/json-c-static.dir/linkhash.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/json-c-static.dir/linkhash.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/linkhash.c" > CMakeFiles/json-c-static.dir/linkhash.c.i

CMakeFiles/json-c-static.dir/linkhash.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/json-c-static.dir/linkhash.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/linkhash.c" -o CMakeFiles/json-c-static.dir/linkhash.c.s

CMakeFiles/json-c-static.dir/printbuf.c.o: CMakeFiles/json-c-static.dir/flags.make
CMakeFiles/json-c-static.dir/printbuf.c.o: /Users/rgriffith/Documents/Computer\ Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/printbuf.c
CMakeFiles/json-c-static.dir/printbuf.c.o: CMakeFiles/json-c-static.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c-build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_10) "Building C object CMakeFiles/json-c-static.dir/printbuf.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/json-c-static.dir/printbuf.c.o -MF CMakeFiles/json-c-static.dir/printbuf.c.o.d -o CMakeFiles/json-c-static.dir/printbuf.c.o -c "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/printbuf.c"

CMakeFiles/json-c-static.dir/printbuf.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/json-c-static.dir/printbuf.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/printbuf.c" > CMakeFiles/json-c-static.dir/printbuf.c.i

CMakeFiles/json-c-static.dir/printbuf.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/json-c-static.dir/printbuf.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/printbuf.c" -o CMakeFiles/json-c-static.dir/printbuf.c.s

CMakeFiles/json-c-static.dir/random_seed.c.o: CMakeFiles/json-c-static.dir/flags.make
CMakeFiles/json-c-static.dir/random_seed.c.o: /Users/rgriffith/Documents/Computer\ Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/random_seed.c
CMakeFiles/json-c-static.dir/random_seed.c.o: CMakeFiles/json-c-static.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c-build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_11) "Building C object CMakeFiles/json-c-static.dir/random_seed.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/json-c-static.dir/random_seed.c.o -MF CMakeFiles/json-c-static.dir/random_seed.c.o.d -o CMakeFiles/json-c-static.dir/random_seed.c.o -c "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/random_seed.c"

CMakeFiles/json-c-static.dir/random_seed.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/json-c-static.dir/random_seed.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/random_seed.c" > CMakeFiles/json-c-static.dir/random_seed.c.i

CMakeFiles/json-c-static.dir/random_seed.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/json-c-static.dir/random_seed.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/random_seed.c" -o CMakeFiles/json-c-static.dir/random_seed.c.s

CMakeFiles/json-c-static.dir/strerror_override.c.o: CMakeFiles/json-c-static.dir/flags.make
CMakeFiles/json-c-static.dir/strerror_override.c.o: /Users/rgriffith/Documents/Computer\ Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/strerror_override.c
CMakeFiles/json-c-static.dir/strerror_override.c.o: CMakeFiles/json-c-static.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c-build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_12) "Building C object CMakeFiles/json-c-static.dir/strerror_override.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/json-c-static.dir/strerror_override.c.o -MF CMakeFiles/json-c-static.dir/strerror_override.c.o.d -o CMakeFiles/json-c-static.dir/strerror_override.c.o -c "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/strerror_override.c"

CMakeFiles/json-c-static.dir/strerror_override.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/json-c-static.dir/strerror_override.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/strerror_override.c" > CMakeFiles/json-c-static.dir/strerror_override.c.i

CMakeFiles/json-c-static.dir/strerror_override.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/json-c-static.dir/strerror_override.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/strerror_override.c" -o CMakeFiles/json-c-static.dir/strerror_override.c.s

CMakeFiles/json-c-static.dir/json_pointer.c.o: CMakeFiles/json-c-static.dir/flags.make
CMakeFiles/json-c-static.dir/json_pointer.c.o: /Users/rgriffith/Documents/Computer\ Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/json_pointer.c
CMakeFiles/json-c-static.dir/json_pointer.c.o: CMakeFiles/json-c-static.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c-build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_13) "Building C object CMakeFiles/json-c-static.dir/json_pointer.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/json-c-static.dir/json_pointer.c.o -MF CMakeFiles/json-c-static.dir/json_pointer.c.o.d -o CMakeFiles/json-c-static.dir/json_pointer.c.o -c "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/json_pointer.c"

CMakeFiles/json-c-static.dir/json_pointer.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/json-c-static.dir/json_pointer.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/json_pointer.c" > CMakeFiles/json-c-static.dir/json_pointer.c.i

CMakeFiles/json-c-static.dir/json_pointer.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/json-c-static.dir/json_pointer.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c/json_pointer.c" -o CMakeFiles/json-c-static.dir/json_pointer.c.s

# Object files for target json-c-static
json__c__static_OBJECTS = \
"CMakeFiles/json-c-static.dir/arraylist.c.o" \
"CMakeFiles/json-c-static.dir/debug.c.o" \
"CMakeFiles/json-c-static.dir/json_c_version.c.o" \
"CMakeFiles/json-c-static.dir/json_object.c.o" \
"CMakeFiles/json-c-static.dir/json_object_iterator.c.o" \
"CMakeFiles/json-c-static.dir/json_tokener.c.o" \
"CMakeFiles/json-c-static.dir/json_util.c.o" \
"CMakeFiles/json-c-static.dir/json_visit.c.o" \
"CMakeFiles/json-c-static.dir/linkhash.c.o" \
"CMakeFiles/json-c-static.dir/printbuf.c.o" \
"CMakeFiles/json-c-static.dir/random_seed.c.o" \
"CMakeFiles/json-c-static.dir/strerror_override.c.o" \
"CMakeFiles/json-c-static.dir/json_pointer.c.o"

# External object files for target json-c-static
json__c__static_EXTERNAL_OBJECTS =

libjson-c.a: CMakeFiles/json-c-static.dir/arraylist.c.o
libjson-c.a: CMakeFiles/json-c-static.dir/debug.c.o
libjson-c.a: CMakeFiles/json-c-static.dir/json_c_version.c.o
libjson-c.a: CMakeFiles/json-c-static.dir/json_object.c.o
libjson-c.a: CMakeFiles/json-c-static.dir/json_object_iterator.c.o
libjson-c.a: CMakeFiles/json-c-static.dir/json_tokener.c.o
libjson-c.a: CMakeFiles/json-c-static.dir/json_util.c.o
libjson-c.a: CMakeFiles/json-c-static.dir/json_visit.c.o
libjson-c.a: CMakeFiles/json-c-static.dir/linkhash.c.o
libjson-c.a: CMakeFiles/json-c-static.dir/printbuf.c.o
libjson-c.a: CMakeFiles/json-c-static.dir/random_seed.c.o
libjson-c.a: CMakeFiles/json-c-static.dir/strerror_override.c.o
libjson-c.a: CMakeFiles/json-c-static.dir/json_pointer.c.o
libjson-c.a: CMakeFiles/json-c-static.dir/build.make
libjson-c.a: CMakeFiles/json-c-static.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir="/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c-build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_14) "Linking C static library libjson-c.a"
	$(CMAKE_COMMAND) -P CMakeFiles/json-c-static.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/json-c-static.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/json-c-static.dir/build: libjson-c.a
.PHONY : CMakeFiles/json-c-static.dir/build

CMakeFiles/json-c-static.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/json-c-static.dir/cmake_clean.cmake
.PHONY : CMakeFiles/json-c-static.dir/clean

CMakeFiles/json-c-static.dir/depend:
	cd "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c-build" && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c" "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c" "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c-build" "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c-build" "/Users/rgriffith/Documents/Computer Networks/CSE30264-SP22-ComputerNetworks/project2/json-c-build/CMakeFiles/json-c-static.dir/DependInfo.cmake" --color=$(COLOR)
.PHONY : CMakeFiles/json-c-static.dir/depend

