#!/bin/bash -e

# https://github.com/ethz-asl/aslam_android_viz/blob/master/thirdparty/make.sh

THIRDPARTY=$(dirname $0)

cd $THIRDPARTY
git clone git://vtk.org/VES.git
cd VES/Apps/Android/CMakeBuild
cmake -P configure.cmake
cd build
make -j8

# cd $THIRDPARTY
# cd VES/Apps/Android/Kiwi
# ./configure_cmake.sh
# ./configure_ant.sh
# ./compile.sh
