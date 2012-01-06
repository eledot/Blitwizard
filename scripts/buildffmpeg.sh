#!/bin/bash

# This is for building FFmpeg for Windows on a Linux machine.
# It doesn't serve any other purpose (like building FFmpeg on Linux for Linux).

cd ..
cd src/ffmpeg/

./configure --disable-static --enable-shared --enable-version3 --enable-memalign-hack --arch=x86 --target-os=mingw32 --cross-prefix=i686-pc-mingw32- --disable-gpl --disable-devices --disable-libgsm --disable-protocols --disable-bzlib --disable-zlib --disable-network --disable-ffmpeg --disable-avconv --disable-ffplay --disable-ffserver --disable-ffprobe --disable-avdevice --disable-avfilter --disable-swresample --disable-swscale --disable-postproc

make

