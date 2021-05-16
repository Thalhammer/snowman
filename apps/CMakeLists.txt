add_executable(enroll_cut
    helper.cpp
    enroll_cut.cpp
)
target_include_directories(enroll_cut PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(enroll_cut PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../lib/)
target_link_libraries(enroll_cut snowboy)