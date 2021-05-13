cmake_minimum_required(VERSION 3.10)
project(snowboy)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

#option(SNOWBOY_CXX11_COMPAT "Build library with C++11 strings disable to be binary compatible with the original release." OFF)
set(SNOWBOY_CXX11_COMPAT ON)

if(SNOWBOY_CXX11_COMPAT)
add_compile_options(-D_GLIBCXX_USE_CXX11_ABI=0)
endif()

add_compile_options(-std=c++0x -Wall -Wno-sign-compare -Wno-unused-local-typedefs -Winit-self -rdynamic)
add_compile_options(-DHAVE_POSIX_MEMALIGN -I. -fno-omit-frame-pointer -fPIC -msse -msse2)
add_compile_options("$<$<CONFIG:DEBUG>:-fsanitize=address>")
add_link_options(-rdynamic)
add_link_options("$<$<CONFIG:DEBUG>:-fsanitize=address>")

add_subdirectory(lib)
add_subdirectory(test)