#!/bin/bash -e

# Almost all of these libraries
# are for building libcvd.

aptitude -PvVR install \
    build-essential    \
    libboost-all-dev   \
    libjpeg-dev        \
    libpng-dev         \
    libtiff-dev        \
    liblapack-dev      \
    libblitz-dev       \
    libv4l-dev         \
    mesa-common-dev    \
    libgl1-mesa-dev    \
    libglu1-mesa-dev   \
    mesa-utils         \
    freeglut3-dev      \
    ffmpeg             \
    libffms2-dev       \
    libavcodec-dev     \
    libavdevice-dev    \
    libavfilter-dev    \
    libavformat-dev    \
    libdc1394-22-dev   \

