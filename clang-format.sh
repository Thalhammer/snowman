#!/bin/bash
clang-format -i `find ./lib -name "*.cpp"` `find ./lib -name "*.h"` `find ./lib -name "*.c"`
clang-format -i `find ./apps -name "*.cpp"` `find ./apps -name "*.h"` `find ./apps -name "*.c"`
clang-format -i `find ./test -name "*.cpp"` `find ./test -name "*.h"` `find ./test -name "*.c"`