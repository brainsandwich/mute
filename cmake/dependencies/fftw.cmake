# set(LOCAL_EXT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ext/local")

# set(BUILD_TESTS OFF)
# set(BUILD_SHARED_LIBS OFF)
# set(ENABLE_OPENMP ON)
# set(ENABLE_THREADS ON)
# set(WITH_COMBINED_THREADS ON)
# set(ENABLE_FLOAT ON)
# set(ENABLE_SSE ON)
# set(ENABLE_SSE2 ON)
# set(ENABLE_AVX ON)
# set(ENABLE_AVX2 ON)
# add_subdirectory(${LOCAL_EXT_DIR}/fftw-3.3.10 ${FETCHCONTENT_BASE_DIR}/fftw)

# add_library(fftw::fftw3 ALIAS FFTW3::fftw3f)
# unset(LOCAL_EXT_DIR)




include(cmake/CPM.cmake)

set(CPM_fftw_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/ext/local/fftw-3.3.10")
CPMAddPackage(
    NAME fftw
    VERSION 3.3.10
    OPTIONS
        "BUILD_TESTS OFF"
        "BUILD_SHARED_LIBS OFF"
        "ENABLE_OPENMP ON"
        "ENABLE_THREADS ON"
        "WITH_COMBINED_THREADS ON"
        "ENABLE_FLOAT ON"
        "ENABLE_SSE ON"
        "ENABLE_SSE2 ON"
        "ENABLE_AVX ON"
        "ENABLE_AVX2 ON"
)
if (fftw_ADDED)
    target_compile_options(fftw3f PRIVATE -Wno-unused-command-line-argument)
    add_library(fftw::fftw3 ALIAS fftw3f)
endif()