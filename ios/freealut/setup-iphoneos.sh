#!/bin/sh

# This script is used for compiling freealut for iOS device, other version not testes
# Modify variables if needed

export SBROOT=""

if [ ! -d "$SBROOT" ]; then
    echo "smartbody trunk location $SBROOT not set correctly"
else
    mkdir build
    cd build
    cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-iphoneos.cmake ..
    make
    cd ..
fi

cp ./*.a $SBROOT/ios/libs/iphoneos