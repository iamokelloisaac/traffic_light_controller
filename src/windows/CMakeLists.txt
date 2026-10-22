cmake_minimum_required(VERSION 3.20)
project(traffic_light_controller C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Windows (MSVC) does not understand GCC warning flags like -Wall or -Wextra.
if(NOT MSVC)
    add_compile_options(
            -Wall
            -Wextra
            -Wpedantic
            -Werror
            -Wshadow
            -Wconversion
            -Wsign-conversion
    )
else()
    # MSVC equivalent for strict warnings
    add_compile_options(/W4 /WX)
endif()

add_executable(traffic_light_controller
        src/main.c
        src/traffic_light.c
        src/state_machine.c
        src/timing.c
        src/input.c
        src/ui.c
        src/terminal.c
        src/platform.c
)

target_include_directories(traffic_light_controller
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link threads only on non-Windows platforms.
if(NOT WIN32)
    find_package(Threads REQUIRED)
    target_link_libraries(traffic_light_controller PRIVATE Threads::Threads)
endif()