#!/bin/sh

# This script is used for compiling clapack for iphone simulator, other version not tested
# Modify variables if needed

export SBROOT=""

if [ ! -d "$SBROOT" ]; then
echo "smartbody trunk location $SBROOT not set correctly"
else
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-iphonesimulator.cmake ..
make
make
cp BLAS/SRC/libblas.a $SBROOT/ios/libs/iphonesimulator
cp SRC/liblapack.a $SBROOT/ios/libs/iphonesimulator
cp F2CLIBS/libf2c/libf2c.a $SBROOT/ios/libs/iphonesimulator
fi

