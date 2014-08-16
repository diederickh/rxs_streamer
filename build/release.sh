#!/bin/sh

if [ ! -d build.release ] ; then 
    mkdir build.release
fi

cd build.release
#cmake -DCMAKE_BUILD_TYPE=Release ../ 
cmake -DCMAKE_BUILD_TYPE=Debug ../ 
cmake --build . --target install

if [ "$(uname)" == "Darwin" ] ; then 
    cd ./../../install/mac-clang-x86_64/bin/
else
    cd ./../../install/linux-gcc-x86_64/bin/
fi

#./test_vpx
#./test_glfw_player
#./test_packets
#./test_jitter
#./test_control
#./test_webcam_streamer
#./test_stun
#./test_fec
#./test_nanomsg_client
#./test_nanomsg_server
#./test_signal_server
#./test_signal_client
#./test_signal_redis
./test_streamer
