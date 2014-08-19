cmake_minimum_required(VERSION 2.8)

include(${CMAKE_CURRENT_LIST_DIR}/../extern/webrtc/build/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/Triplet.cmake)

set(app app)
set(sd ${CMAKE_CURRENT_LIST_DIR}/../src/)
set(webrtc_bd ${CMAKE_CURRENT_LIST_DIR}/../extern/webrtc/)            # base dir
set(webrtc_extern_id ${webrtc_bd}/extern/${tri_triplet}/include)      # webrtc include files
set(webrtc_extern_ld ${webrtc_bd}/extern/${tri_triplet}/lib)          # webrtc lib files
set(webrtc_install_dir ${webrtc_bd}/install/${tri_triplet}/)          # webrtc lib files
set(webrtc_id ${webrtc_bd}/include/)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set(app "${app}_debug")
endif()

include_directories(
  ${CMAKE_CURRENT_LIST_DIR}/../src
  ${CMAKE_CURRENT_LIST_DIR}/../include
  ${extern_include_dir}
  ${CMAKE_CURRENT_LIST_DIR}/../extern/tinylib/src
  ${webrtc_extern_id}
  ${webrtc_id}
)

set(streamer_sources
  ${sd}/streamer/Buffer.cpp
  ${sd}/streamer/Loop.cpp
  ${sd}/streamer/Socket.cpp
  ${sd}/streamer/SocketUDP.cpp
  ${sd}/streamer/Sender.cpp
  ${sd}/streamer/Receiver.cpp
  ${sd}/streamer/Controller.cpp

  # Compile the webrtc source in the streamer
  ${webrtc_sources}
)

if(UNIX AND NOT APPLE)
  list(APPEND app_sources ${extern_source_dir}/GLXW/glxw.c)
  message(STATUS "Adding glxw.c for GL-function loading.")
  message(${app_sources})
endif()

if(APPLE)

  find_library(fr_corefoundation CoreFoundation)
  find_library(fr_avfoundation AVFoundation)
  find_library(fr_coremedia CoreMedia)
  find_library(fr_cocoa Cocoa)
  find_library(fr_opengl OpenGL)
  find_library(fr_iokit IOKit)
  find_library(fr_corevideo CoreVideo)
  find_library(fr_opencl OpenCL)
  find_library(fr_audio AudioUnit)
  find_library(fr_caudio CoreAudio)
  find_library(fr_audio_toolbox AudioToolbox)

  set(app_libs
#    ${extern_lib_dir}/libvpx.a
    ${extern_lib_dir}/libuv.a
    ${extern_lib_dir}/libpng.a
    ${extern_lib_dir}/libz.a
    ${extern_lib_dir}/libglfw3.a
    ${extern_lib_dir}/libvideocapture.a
    ${extern_lib_dir}/libyuv.a
    ${fr_corefoundation}               # uv
    ${fr_cocoa}                        # uv
    ${fr_iokit}                        # uv
    ${fr_opengl}                       # glfw
    ${fr_corevideo}                    # glfw
    ${fr_avfoundation}                 # videocapture
    ${fr_coremedia}                    # videocapture

    # webrtc 
    ${webrtc_libs}
    ${webrtc_extern_ld}/libvideogenerator.a    # for video stream test
    ${webrtc_extern_ld}/libvpx.a               # video encoder
    )

elseif(UNIX)

  set(debug_flag "")

  if(CMAKE_BUILD_TYPE STREQUAL Debug)
    set(debug_flag "d")
  endif()

  set(app_libs
    ${extern_lib_dir}/libvpx${debug_flag}.a
    ${extern_lib_dir}/libglfw3.a
    ${extern_lib_dir}/libuv.a
    ${extern_lib_dir}/libyuv.a  # for webcam conversion
    ${extern_lib_dir}/libpng.a
    ${extern_lib_dir}/libvideocapture.a
    ${extern_lib_dir}/libhiredis.a     # for signaling
    udev         # libvideocapture
    GL
    X11
    Xxf86vm
    Xrandr
    pthread
    dl
    Xi
    asound
    z
    m
    )
endif()

add_library(streamer ${streamer_sources})
install(TARGETS streamer ARCHIVE DESTINATION lib)

macro(create_test name)
  add_executable(test_rxs_${name} ${sd}/examples/test_rxs_${name}.cpp ${app_sources})
  target_link_libraries(test_rxs_${name} ${app_libs} streamer)
  install(TARGETS test_rxs_${name} DESTINATION bin)
endmacro()

create_test(receiver)            # receive the encoded video
create_test(controller)          # proxy input to multiple outputs
create_test(sender)              # send video to the controller
create_test(streamer)            # general test for the api
create_test(video_sender)        # encodes some test video and sends it to the controller.
create_test(video_receiver)      # receives video from the controller and writes it to a .ivf file (optional)









