#!/bin/sh

# This script is used for compiling ode-0.11.1 for iphone simulator, other version not tested
# Modify variables if needed

export SBROOT=""
export PREFIX="$SBROOT/ios/ode/iphonesimulator"
export ROOTDIR="/Developer"
export PLATFORM="iPhoneSimulator"
export ARCH="-arch i386"
export MIN_VERSION="4.1"
export MAX_VERSION="4.3"
export OPT="3"

export DEVROOT=/Developer/Platforms/iPhoneSimulator.platform/Developer
export SDKROOT=$DEVROOT/SDKs/iPhoneSimulator4.3.sdk
export CC=$DEVROOT/usr/bin/gcc
export LD=$DEVROOT/usr/bin/ld
export CPP=$DEVROOT/usr/bin/cpp
export CXX=$DEVROOT/usr/bin/g++
export AR=$DEVROOT/usr/bin/ar
export LIBTOOL=$DEVROOT/usr/bin/libtool
export AS=$DEVROOT/usr/bin/as
export NM=$DEVROOT/usr/bin/nm
export CXXCPP=$DEVROOT/usr/bin/cpp
export RANLIB=$DEVROOT/usr/bin/ranlib
export OPTFLAG="-O${OPT}"
export COMMONFLAGS="${ARCH} -pipe $OPTFLAG -gdwarf-2 -no-cpp-precomp -mthumb -isysroot ${SDKROOT} -miphoneos-version-min=${MIN_VERSION}"
export CFLAGS="-arch i386 -pipe -no-cpp-precomp -isysroot $SDKROOT -miphoneos-version-min=4.1 -I$SDKROOT/usr/include/"
export CXXFLAGS="$CFLAGS"
export LDFLAGS="-L$SDKROOT/usr/lib/ -L$SDKROOT/usr/lib/system"

if [ ! -d "$SBROOT" ]; then
    echo "smartbody trunk location $SBROOT not set correctly"
else
    ./configure --host=i386-apple-darwin --with-trimesh=none --with-drawstuff=none --with-pic --prefix=$PREFIX
    make clean
    make
    make install
fi