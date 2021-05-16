add_executable(cut
    helper.cpp
    cut.cpp
)
target_include_directories(cut PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cut snowboy)

add_executable(enroll
    helper.cpp
    enroll.cpp
)
target_include_directories(enroll PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(enroll crypto snowboy)