#!/bin/sh

# This script is used for compiling ode-0.11.1 for iphone device, other version not tested
# Modify PREFIX
# Modify variables if needed

export SBROOT=""
export ARMVERSION=7
export PREFIX="$SBROOT/ios/ode/iphoneos"
export DEVROOT=/Developer/Platforms/iPhoneOS.platform/Developer
export SDKROOT=$DEVROOT/SDKs/iPhoneOS4.3.sdk
export CFLAGS="-arch armv$ARMVERSION -pipe -no-cpp-precomp -isysroot$SDKROOT -miphoneos-version-min=4.1 -I$SDKROOT/usr/include/"
export CPPFLAGS="$CFLAGS"
export CXXFLAGS="$CFLAGS"
export LDFLAGS="-L$SDKROOT/usr/lib/"
export CPP="$DEVROOT/usr/bin/cpp-4.2"
export CXX="$DEVROOT/usr/bin/g++-4.2"
export CC="$DEVROOT/usr/bin/gcc-4.2"
export LD="$DEVROOT/usr/bin/ld"
echo "ac_cv_func_realloc_0_nonnull=yes\nac_cv_func_malloc_0_nonnull=yes">arm-apple.cache
export RANLIB="$DEVROOT/usr/bin/ranlib"

if [ ! -d "$SBROOT" ]; then
    echo "smartbody trunk location $SBROOT not set correctly"
else
    ./configure --with-drawstuff=none --with-pic --host="arm-apple-darwin" --prefix=$PREFIX --cache-file=arm-apple.cache --with-trimesh=gimpact
    make clean
    make
    make install
fi