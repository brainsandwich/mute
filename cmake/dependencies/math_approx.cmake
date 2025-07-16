include(CPM)

CPMAddPackage("https://github.com/Chowdhury-DSP/math_approx.git#713cab874ac7168c345a147c293f4eac200a5948")

if (math_approx_ADDED)
    add_library(math_approx::math_approx ALIAS math_approx)
endif()