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

if(SNOWMAN_BUILD_APPS_STATIC)
    IF("${CMAKE_BUILD_TYPE}" MATCHES "^(Debug)\$")
        message(WARNING "Ignored BUILD_APPS_STATIC because it is incompatible with debug mode")
    else()
        message(STATUS "Linking apps statically")
        target_link_libraries(cut -static)
        target_link_libraries(enroll -static)
    endif()
endif()

if(LTOAvailable)
    message(STATUS "LTO enabled for apps")
    set_property(TARGET cut PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    set_property(TARGET enroll PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()