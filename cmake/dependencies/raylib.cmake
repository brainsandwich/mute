include(cmake/CPM.cmake)

CPMAddPackage(
    NAME raylib
    VERSION 5.0
    GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
    GIT_TAG "5.0"
    # DOWNLOAD_ONLY ON
)
if (raylib_ADDED)
    add_library(raylib::raylib ALIAS raylib)
endif()