#!/bin/bash -eu
HERE=$(cd $(dirname $0); pwd)
BUILD=$HERE/build
if [ ! -d $BUILD ]; then
    mkdir $BUILD
fi
cd $BUILD
CACHE=CMakeCache.txt
if [ -e $CACHE ]; then
    rm $CACHE
fi
cmake ..
make -j
