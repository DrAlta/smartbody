#!/bin/sh

# This script is used for compiling apr-1.3.* for iphone device, apr-1.4.5 doesn't work
# Modify PREFIX
# Modify variables if needed

export SBROOT=""
export ARMVERSION=7
export PREFIX="$SBROOT/ios/activemq/apr/iphoneos"
export DEVROOT=/Developer/Platforms/iPhoneOS.platform/Developer
export SDKROOT=$DEVROOT/SDKs/iPhoneOS4.3.sdk
export CFLAGS="-arch armv$ARMVERSION -pipe -no-cpp-precomp -isysroot $SDKROOT -miphoneos-version-min=4.1 -I$SDKROOT/usr/include/"
export CPPFLAGS="$CFLAGS"
export CXXFLAGS="$CFLAGS"
export LDFLAGS="-L$SDKROOT/usr/lib/ -L$SDKROOT/usr/lib/system"
export CPP="$DEVROOT/usr/bin/cpp-4.2"
export CXX="$DEVROOT/usr/bin/g++-4.2"
export CC="$DEVROOT/usr/bin/gcc-4.2"
export LD="$DEVROOT/usr/bin/ld"


if [ ! -d "$SBROOT" ]; then
    echo "smartbody trunk location $SBROOT not set correctly"
else
    ./configure \
    --prefix=$PREFIX \
    --host="arm-apple-darwin" \
    --disable-shared \
    ac_cv_file__dev_zero="yes" \
    ac_cv_func_setpgrp_void="yes" \
    apr_cv_process_shared_works="yes" \
    apr_cv_mutex_robust_shared="no" \
    apr_cv_tcp_nodelay_with_cork="yes" \
    ac_cv_sizeof_struct_iovec="8" \
    apr_cv_mutex_recursive="yes" \
    --disable-dso \
    --enable-threads

    make clean
    make
    make install
fi

cp $PREFIX/lib/*.a $SBROOT/ios/libs/iphoneos