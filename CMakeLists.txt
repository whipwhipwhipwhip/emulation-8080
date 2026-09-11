cmake_minimum_required(VERSION 3.10)
project(SpaceInvadersTest VERSION 1.0)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Use pkg-config to find SDL2 and SDL2_mixer
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_MIXER REQUIRED sdl2_mixer)

set(SOURCES
    src/driver.c
    src/disassembler.c
    src/emulator.c
    src/graphics.c
    src/io_devices.c
    src/machine.c
    src/memory.c
)

# Add an executable with the above sources
add_executable(${PROJECT_NAME} ${SOURCES})

# Include/library directories and flags all come from pkg-config, so this
# works regardless of Homebrew prefix (Apple Silicon vs Intel) or OS.
target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_MIXER_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
)

target_link_directories(${PROJECT_NAME}
    PRIVATE
    ${SDL2_LIBRARY_DIRS}
    ${SDL2_MIXER_LIBRARY_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    ${SDL2_LIBRARIES}
    ${SDL2_MIXER_LIBRARIES}
)

target_compile_options(${PROJECT_NAME} PRIVATE
    ${SDL2_CFLAGS_OTHER}
    ${SDL2_MIXER_CFLAGS_OTHER}
)

# Standalone CPU-only diagnostics runner (no SDL): runs an 8080 exerciser
# .COM file (e.g. cpu_tests/i8080-core-master/CPUTEST.COM) against the core.
add_executable(Diag8080
    src/disassembler.c
    src/emulator.c
    src/memory.c
    src/run_cpu_test.c
)

target_include_directories(Diag8080
    PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

# The exerciser ROMs run tens of millions of interpreted instructions
# (e.g. CPUTEST.COM's built-in timing-calibration loop) - -O2 keeps that
# a matter of seconds rather than minutes despite CMAKE_BUILD_TYPE Debug.
target_compile_options(Diag8080 PRIVATE -O2)