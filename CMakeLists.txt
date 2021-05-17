cmake_minimum_required(VERSION 3.12)
project(snowman VERSION 1.0.0 DESCRIPTION "Snowman hotword detection library")
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

option(SNOWMAN_CXX11_COMPAT "Build library with C++11 strings disabled to be binary compatible with the original release." ON)
option(SNOWMAN_BUILD_APPS "Build helper applications like enroll or cut" ON)
option(SNOWMAN_BUILD_TESTS "Build unit tests (requires gtest and openssl)" ON)
option(SNOWMAN_BUILD_SHARED "Build library as a shared library instead of a static library" OFF)
# According to steam ~99.17% of users have at least ssse3
# and 100% have SSE, SSE2 and SSE3, so this is on by default
option(SNOWMAN_BUILD_WITH_SSE3 "Enable sse3 optimizations" ON)
# ~98.3%, so this is on by default as well
option(SNOWMAN_BUILD_WITH_SSE4 "Enable sse4 optimizations" ON)
# ~94.7%
option(SNOWMAN_BUILD_WITH_AVX "Enable avx optimizations" OFF)
# ~82%
option(SNOWMAN_BUILD_WITH_AVX2 "Enable avx2 optimizations" OFF)
option(SNOWMAN_BUILD_NATIVE "Build library for the current cpu. This makes sure it uses every instruction set available, but the resulting binary probably won't run on older hardware." OFF)

add_compile_options("$<$<CONFIG:DEBUG>:-fsanitize=address>")
add_link_options("$<$<CONFIG:DEBUG>:-fsanitize=address>")

if(SNOWMAN_CXX11_COMPAT)
add_compile_options(-D_GLIBCXX_USE_CXX11_ABI=0)
endif()

add_subdirectory(lib)
if(SNOWMAN_BUILD_APPS)
    add_subdirectory(apps)
endif()
if(SNOWMAN_BUILD_TESTS)
    add_subdirectory(test)
endif()