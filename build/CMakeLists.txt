cmake_minimum_required(VERSION 2.8)

include(${CMAKE_CURRENT_LIST_DIR}/Triplet.cmake)

set(app app)
set(sd ${CMAKE_CURRENT_LIST_DIR}/../src/)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set(app "${app}_debug")
endif()

include_directories(
  ${CMAKE_CURRENT_LIST_DIR}/../src
  ${CMAKE_CURRENT_LIST_DIR}/../include
  ${extern_include_dir}
  ${CMAKE_CURRENT_LIST_DIR}/../extern/tinylib/src
)

set(rxs_streamer_sources
  ${sd}/rxs_streamer/rxs_generator.c
  ${sd}/rxs_streamer/rxs_encoder.c
  ${sd}/rxs_streamer/rxs_decoder.c
  ${sd}/rxs_streamer/rxs_packetizer.c
  ${sd}/rxs_streamer/rxs_depacketizer.c
  ${sd}/rxs_streamer/rxs_sender.c
  ${sd}/rxs_streamer/rxs_receiver.c
  ${sd}/rxs_streamer/rxs_ivf.c
  ${sd}/rxs_streamer/rxs_packets.c
  ${sd}/rxs_streamer/rxs_jitter.c
  ${sd}/rxs_streamer/rxs_control.c
  ${sd}/rxs_streamer/rxs_reconstruct.c
)

if(UNIX AND NOT APPLE)
  list(APPEND app_sources ${extern_source_dir}/GLXW/glxw.c)
  message(STATUS "Adding glxw.c for GL-function loading.")
  message(${app_sources})
endif()

if(APPLE)

  find_library(fr_corefoundation CoreFoundation)
  find_library(fr_cocoa Cocoa)
  find_library(fr_opengl OpenGL)
  find_library(fr_iokit IOKit)
  find_library(fr_corevideo CoreVideo)
  find_library(fr_opencl OpenCL)
  find_library(fr_audio AudioUnit)
  find_library(fr_caudio CoreAudio)
  find_library(fr_audio_toolbox AudioToolbox)

  set(app_libs
    ${extern_lib_dir}/libvpx.a
    ${extern_lib_dir}/libuv.a
    ${extern_lib_dir}/libpng.a
    ${extern_lib_dir}/libz.a
    ${extern_lib_dir}/libglfw3.a
    ${fr_corefoundation}  # uv
    ${fr_cocoa}           # uv
    ${fr_iokit}           # uv
    ${fr_opengl}          # glfw
    ${fr_corevideo}       # glfw
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
    ${extern_lib_dir}/libpng.a
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

# rxs_streamer library
add_library(rxs_streamer ${rxs_streamer_sources})
install(TARGETS rxs_streamer ARCHIVE DESTINATION lib)

# test vpx sender
add_executable(test_vpx ${sd}/examples/test_vpx.c ${app_sources})
target_link_libraries(test_vpx rxs_streamer ${app_libs})
install(TARGETS test_vpx DESTINATION bin)

# test glfw player
add_executable(test_glfw_player ${sd}/examples/test_glfw_player.cpp ${app_sources})
target_link_libraries(test_glfw_player rxs_streamer ${app_libs})
install(TARGETS test_glfw_player DESTINATION bin)

# test packets
add_executable(test_packets ${sd}/examples/test_packets.c ${app_sources})
target_link_libraries(test_packets rxs_streamer ${app_libs})
install(TARGETS test_packets DESTINATION bin)

# test jitter
add_executable(test_jitter ${sd}/examples/test_jitter.c ${app_sources})
target_link_libraries(test_jitter rxs_streamer ${app_libs})
install(TARGETS test_jitter DESTINATION bin)

# test control
add_executable(test_control ${sd}/examples/test_control.c ${app_sources})
target_link_libraries(test_control rxs_streamer ${app_libs})
install(TARGETS test_control DESTINATION bin)









