#!/bin/sh

if [ ! -d build.debug ] ; then 
    mkdir build.debug
fi

cd build.debug
cmake -DCMAKE_BUILD_TYPE=Debug ../ 
cmake --build . --target install
if [ "$(uname)" == "Darwin" ] ; then 
    cd ./../../install/mac-clang-x86_64/bin/
    #lldb ./app_debug  
    lldb ./test_glfw_player
else
    cd ./../../install/linux-gcc-x86_64/bin/
    #gdb ./test_glfw_player
    gdb ./test_jitter
fi
