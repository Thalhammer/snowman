#!/bin/bash
cmake /snowman -DCMAKE_BUILD_TYPE=Release -DSNOWMAN_BUILD_TESTS=OFF \
        -DSNOWMAN_BUILD_APPS=OFF -DSNOWMAN_BUILD_BENCHMARKS=OFF \
        -DSNOWMAN_BUILD_WASM=ON -DSNOWMAN_CXX11_COMPAT=OFF -DSNOWMAN_ENABLE_LTO=OFF
make -j$(nproc) snowboy_wasm
cp wasm/snowboy_wasm.{js,bc} /snowman_output/