#!/bin/sh

if [ ! -d build.debug ] ; then 
    mkdir build.debug
fi

cd build.debug
cmake -DCMAKE_BUILD_TYPE=Debug ../ 
cmake --build . --target install
cd ./../../install/mac-clang-x86_64/bin/
lldb ./app_debug
