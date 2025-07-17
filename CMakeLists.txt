cmake_minimum_required(VERSION 3.22)
project(mute)

# ------------------ DEFAULT COMPILATION FLAGS

# set(MUTE_BUILD_TYPE ${CMAKE_BUILD_TYPE})
# if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
#     set(CMAKE_BUILD_TYPE "Release")
#     set(MUTE_BUILD_TYPE "Debug")
#     message(STATUS "Building only mute in Debug, dependencies in Release")
# endif()

set(CPM_SOURCE_CACHE "${CMAKE_CURRENT_SOURCE_DIR}/ext/fetch" CACHE STRING "")
set(CPM_USE_LOCAL_PACKAGES ON CACHE BOOL "")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
list(APPEND CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies)

# ------------------ SOURCES

add_executable(mute
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cc
    # ${CMAKE_CURRENT_SOURCE_DIR}/src/ui.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mute/driver.cc
    # ${CMAKE_CURRENT_SOURCE_DIR}/src/mute/random.cc
    # ${CMAKE_CURRENT_SOURCE_DIR}/src/mute/time.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mute/debug.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mute/miniaudio.impl.cc
    # ${CMAKE_CURRENT_SOURCE_DIR}/src/mute/modules/oscillator.cc
)
set_target_properties(mute PROPERTIES FOLDER "mute")
target_compile_features(mute PUBLIC cxx_std_23)
target_compile_options(mute PUBLIC --std=c++2c)
target_include_directories(mute PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_compile_options(mute PUBLIC -g -gdwarf-4 -O2 -fno-omit-frame-pointer)


include(CMakeParseArguments)
function(target_warnings TARGET SCOPE)
    set(options
        ALL
        ERROR
    )
    set(oneValueArgs)
    set(multiValueArgs)
    cmake_parse_arguments(PARSE_ARGV 0 arg "${options}" "${oneValueArgs}" "${multiValueArgs}")

    if (arg_ALL)
        if (MSVC)
            target_compile_options(${TARGET} ${SCOPE} /W4)
        else()
            target_compile_options(${TARGET} ${SCOPE} -Wall -Wextra -Wpedantic)
        endif()
    endif()
    
    if (arg_ERROR)
        if (MSVC)
            target_compile_options(${TARGET} ${SCOPE} /WX)
        else()
            target_compile_options(${TARGET} ${SCOPE} -Werror)
        endif()
    endif()
endfunction()

target_warnings(mute PUBLIC ALL ERROR)
if (NOT MSVC)
    target_compile_options(mute PUBLIC
        -Wno-unused-parameter
        -Wno-gnu-anonymous-struct
        -Wno-nested-anon-types
        -Wno-unused-const-variable
        -Wno-missing-braces
        -Wno-missing-field-initializers
        -Wno-unused-variable
        -Wno-unused-but-set-variable
        -Wno-unused-function)
endif()

# set_target_properties(mute PROPERTIES CMAKE_BUILD_TYPE ${MUTE_BUILD_TYPE})

# ------------------ DEPENDENCIES

include(fmt)
include(miniaudio)
include(libremidi)
include(raylib)
# include(fftw)
include(math_approx)
# include(entt)
# include(flecs)

target_compile_definitions(mute
    PUBLIC
        MUTE_USE_MINIAUDIO
        MUTE_USE_FMT
        MUTE_USE_LIBREMIDI
        MUTE_USE_RAYLIB
        # MUTE_USE_FFTW
        MUTE_USE_MATH_APPROX
        # MUTE_USE_ENTT
        # MUTE_USE_FLECS
)
target_link_libraries(mute
    PUBLIC
        fmt::fmt
        miniaudio::miniaudio
        libremidi::libremidi
        raylib::raylib
        # fftw::fftw3
        math_approx::math_approx
        # EnTT::EnTT
        # flecs::flecs
)

if (APPLE)
    target_link_libraries(mute
        PUBLIC
            "-framework IOKit"
            "-framework Cocoa"
            "-framework OpenGL"
    )
endif()

# ------------------ ASSETS

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_CURRENT_SOURCE_DIR}/assets" "${CMAKE_CURRENT_BINARY_DIR}/assets"
    DEPENDS ${destination}
    COMMENT "symbolic link resources folder from ${CMAKE_CURRENT_SOURCE_DIR}/assets => ${CMAKE_CURRENT_BINARY_DIR}/assets"
)