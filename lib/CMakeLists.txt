
set(SNOWMAN_SRC
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
    ${CMAKE_CURRENT_SOURCE_DIR}/pipeline-template-cut.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pipeline-vad.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/raw-energy-vad-stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/raw-nnet-vad-stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/snowboy-debug.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/snowboy-detect-c.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/snowboy-detect.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/snowboy-error.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/snowboy-io.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/snowboy-math.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/snowboy-options.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/snowboy-utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/srfft.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tdereverb_x.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/template-container.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/template-detect-stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/template-enroll-stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/universal-detect-stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vad-lib.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vad-state-stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vector-wrapper.cpp
)
set(SNOWMAN_PRIVATE_OPTIONS
    -Wall -Wextra -Winit-self -rdynamic
    -DHAVE_POSIX_MEMALIGN -fno-omit-frame-pointer
)
if(SNOWMAN_BUILD_SHARED)
    add_library(snowman SHARED ${SNOWMAN_SRC})
    set_target_properties(snowman PROPERTIES VERSION ${PROJECT_VERSION})
    set_target_properties(snowman PROPERTIES SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR})
else()
    add_library(snowman STATIC ${SNOWMAN_SRC})
endif()
target_compile_features(snowman PRIVATE cxx_std_11)
target_compile_options(snowman PRIVATE ${SNOWMAN_PRIVATE_OPTIONS})
target_include_directories(snowman PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_directories(snowman PRIVATE /usr/lib/atlas-base)
target_link_libraries(snowman m pthread f77blas cblas lapack_atlas atlas)

if(SNOWMAN_BUILD_WASM)
    add_library(snowman_wasm_lib STATIC ${SNOWMAN_SRC})
    target_compile_features(snowman_wasm_lib PRIVATE cxx_std_11)
    target_compile_options(snowman_wasm_lib PRIVATE ${SNOWMAN_PRIVATE_OPTIONS})
    target_include_directories(snowman_wasm_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    target_include_directories(snowman_wasm_lib PRIVATE /opt/kaldi/tools/CLAPACK)
    target_link_libraries(snowman_wasm_lib dl m rt pthread)
endif()

if(SNOWMAN_BUILD_WITH_SSE3)
target_compile_options(snowman PRIVATE -msse -msse2 -msse3 -mssse3)
endif()
if(SNOWMAN_BUILD_WITH_SSE4)
target_compile_options(snowman PRIVATE -msse4.2)
endif()
if(SNOWMAN_BUILD_WITH_AVX)
target_compile_options(snowman PRIVATE -mavx)
endif()
if(SNOWMAN_BUILD_WITH_AVX2)
target_compile_options(snowman PRIVATE -mavx2)
endif()
if(SNOWMAN_BUILD_NATIVE)
target_compile_options(snowman PRIVATE -march=native -mtune=native)
endif()

add_library(snowboy ALIAS snowman)

if(LTOAvailable)
    set_property(TARGET snowman PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()