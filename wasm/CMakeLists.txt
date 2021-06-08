add_definitions(
    -r 
	-std=c++11
	-Wno-unused-function
    )
include_directories(include ${CMAKE_SOURCE_DIR}/lib)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/wasm)
add_executable(snowboy_wasm ${CMAKE_CURRENT_SOURCE_DIR}/bindings.cpp)
target_link_libraries(snowboy_wasm snowman_wasm_lib)
set(CMAKE_EXECUTABLE_SUFFIX ".bc")

set(LIBS
    /opt/kaldi/tools/gsl/cblas/.libs/libgslcblas.so 
	/opt/kaldi/tools/CLAPACK-WA/F2CLIBS/libf2c.bc 
	/opt/kaldi/tools/CLAPACK-WA/lapack_WA.bc 
	/opt/kaldi/tools/CLAPACK-WA/libcblaswr.bc
	/opt/kaldi/tools/CLAPACK-WA/libcblaswr.bc
    ${CMAKE_BINARY_DIR}/lib/libsnowman_wasm_lib.a
)

set(LINK_FLAGS
	-s SINGLE_FILE=1
	--pre-js ${CMAKE_SOURCE_DIR}/wasm/snowman.pre.js
	--post-js ${CMAKE_SOURCE_DIR}/wasm/snowman.post.js
	--bind
	-std=c++11
	-O3
	-s ALLOW_MEMORY_GROWTH=1
	-s ERROR_ON_UNDEFINED_SYMBOLS=0
	-s MODULARIZE=1
	-s EXPORT_NAME=LoadSnowman
)

add_custom_command(TARGET snowboy_wasm POST_BUILD
    COMMAND ${CMAKE_CXX_COMPILER} -o ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/snowboy_wasm.js ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/snowboy_wasm.bc ${LINK_FLAGS} ${LIBS}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building and bundling"
)

add_custom_command(TARGET snowboy_wasm POST_BUILD
    COMMAND cp ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/snowboy_wasm.js ${CMAKE_CURRENT_SOURCE_DIR}/example
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Copying to example"
) 