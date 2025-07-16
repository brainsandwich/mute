include(cmake/CPM.cmake)

CPMAddPackage(
    NAME raylib
    VERSION 5.5
    GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
    GIT_TAG "5.5"
    OPTIONS
        "USE_AUDIO OFF"
)
if (raylib_ADDED)
    add_library(raylib::raylib ALIAS raylib)
endif()