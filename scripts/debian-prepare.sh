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

