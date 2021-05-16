enable_testing()
include(FetchGoogleTest)
include(GoogleTest)
add_executable(snowboy-test
    helper.cpp
    ClassifyTest.cpp
    DtwTest.cpp
    CutTest.cpp
)
target_include_directories(snowboy-test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(snowboy-test snowboy gtest gtest_main crypto)
gtest_discover_tests(snowboy-test)