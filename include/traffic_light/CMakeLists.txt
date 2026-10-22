# Ask CMake to require version 3.20 or newer. This project uses features
# (like target_compile_definitions) that are reliable from this version on.
cmake_minimum_required(VERSION 3.20)

# Name the project "traffic_light_controller" and tell CMake the only
# language we use is C. (CMake also supports C++, Fortran, etc.)
project(traffic_light_controller C)

# Tell the compiler to use the C23 standard. C23 is the newest version
# of the C language standard.
set(CMAKE_C_STANDARD 23)

# If the compiler does not support C23, fail the build instead of
# silently falling back to an older standard.
set(CMAKE_C_STANDARD_REQUIRED ON)

# Disable compiler-specific extensions (like GCC's extra keywords).
# We want strictly standard C so the code is portable.
set(CMAKE_C_EXTENSIONS OFF)

# Turn on a strict set of compiler warnings. -Werror means "treat every
# warning as a compile error". This forces us to fix warnings immediately
# instead of letting them pile up.
add_compile_options(
        -Wall                 # Warn about common mistakes (unused variables, etc.)
        -Wextra               # Warn about even more things than -Wall
        -Wpedantic            # Warn about non-standard C features
        -Werror               # Convert every warning into a hard error
        -Wshadow              # Warn if a local variable hides another variable
        -Wconversion          # Warn if a type conversion might lose data
        -Wsign-conversion     # Warn if we mix signed and unsigned integers
)

# Define the final executable we want to build. Every .c file in the
# project is listed here. CMake compiles each one separately and then
# links them together into a single program.
add_executable(traffic_light_controller
        ../../src/main.c
        ../../src/traffic_light.c
        ../../src/state_machine.c
        timing.c
        input.c
        ../../src/ui.c
        terminal.c
        ../../src/platform.c
        traffic_light.h
        timing.h
        timing.c
        terminal.h
        terminal.c
        input.h
        input.c
        ui.h
        ../../src/ui.c
        platform.h
        ../../src/platform.c
        state_machine.h
        ../../src/state_machine.c
        ../../src/main.c
        traffic_light.h
)

# Tell the compiler where to find our own header files. This lets us
# write #include "traffic_light/ui.h" instead of a long absolute path.
# PRIVATE means: "only this executable needs this include path; other
# targets in a larger project don't need to know about it."
target_include_directories(traffic_light_controller
        PRIVATE
        ..
)

# Find the system's threading library (pthreads on Linux). We don't
# actually spawn threads in this project yet, but linking it keeps
# the build ready for future extensions and costs nothing.
find_package(Threads REQUIRED)

# Link the threading library into our executable.
target_link_libraries(traffic_light_controller PRIVATE Threads::Threads)