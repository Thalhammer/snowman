
add_library(snowboy_reimpl
    ${CMAKE_CURRENT_SOURCE_DIR}/agc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/audio-lib.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dtw-lib.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/eavesdrop-stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/feat-lib.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fft-stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/framer-stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/frontend-lib.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/frontend-stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gain-control-stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/intercept-stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/license-lib.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/matrix-wrapper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mfcc-stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/nnet-component.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/nnet-lib.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/nnet-stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/noise_suppression_x.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ns3.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pipeline-detect.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pipeline-lib.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pipeline-personal-enroll.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pipeline-vad.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/raw-energy-vad-stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/raw-nnet-vad-stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/snowboy-debug.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/snowboy-detect.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/snowboy-io.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/snowboy-math.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/snowboy-options.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/snowboy-utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tdereverb_x.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/template-container.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/template-detect-stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/template-enroll-stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/universal-detect-stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vad-lib.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vad-state-stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vector-wrapper.cpp
)
target_include_directories(snowboy_reimpl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_directories(snowboy_reimpl PRIVATE /usr/lib/atlas-base)
target_link_libraries(snowboy_reimpl dl m rt pthread f77blas cblas lapack_atlas atlas)

file(TOUCH ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp)
add_library(snowboy ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp)
target_link_libraries(snowboy -Wl,--start-group snowboy_reimpl -Wl,--end-group)
#target_link_libraries(snowboy -Wl,--start-group snowboy_reimpl ${CMAKE_CURRENT_SOURCE_DIR}/../resources/nnet-lib.o -Wl,--end-group)
