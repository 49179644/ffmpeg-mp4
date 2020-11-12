cmake_minimum_required(VERSION 3.15)
project(T_ffmeg)

set(CMAKE_CXX_STANDARD 11)


#link_directories(/usr/lib/x86_64-linux-gnu)
#link_directories(./cmake-build-debug)

link_directories(/usr/lib/x86_64-linux-gnu/)
include_directories(/home/taichen/thrid/package/ffmpge/include)
link_directories(/home/taichen/thrid/package/ffmpge/lib)

add_executable(p4 p4_test.cpp ffmpeg_mp4_dec.cpp)
target_link_libraries(p4 avcodec avformat  swresample  avutil swscale ${CVLIB})

set(CVLIB opencv_imgproc opencv_imgcodecs opencv_video opencv_videoio opencv_highgui opencv_core opencv_calib3d
        opencv_features2d opencv_flann)
add_executable(T_ffmeg main.cpp)
target_link_libraries(T_ffmeg avformat  avcodec  swresample avcodec avutil swscale ${CVLIB})


