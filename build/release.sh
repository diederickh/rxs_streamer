#!/bin/sh

d=${PWD}

if [ ! -d build.release ] ; then 
    mkdir build.release
fi

if [ ! -d ${d}/../extern/webrtc ] ; then 
    mkdir ${d}/../extern/webrtc

    cd ${d}/../extern/webrtc
    git clone git@github.com:roxlu/WebRTC.git .

    cd build

    if [ "$(uname)" == "Darwin" ] ; then 
        ./build_mac_dependencies.sh
    fi

    ./release.sh

    cd ${d}
fi

cd build.release
#cmake -DCMAKE_BUILD_TYPE=Release ../ 
cmake -DCMAKE_BUILD_TYPE=Release ../ 
cmake --build . --target install

if [ "$(uname)" == "Darwin" ] ; then 
    cd ./../../install/mac-clang-x86_64/bin/
else
    cd ./../../install/linux-gcc-x86_64/bin/
fi

#./test_rxs_streamer
#./test_rxs_sender
#./test_rxs_controller
#./test_rxs_receiver
./test_rxs_video_sender
