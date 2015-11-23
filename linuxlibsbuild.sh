#!/bin/sh

MAINDIR=`pwd`
mkdir -p dependencies
cd dependencies

echo "Building boost..."
tar -xvzf boost_1_59_0.tar.gz 
cd boost_1_59_0
./bootstrap.sh --prefix=${MAINDIR}
./bjam -j6
./bjam install

cd ..

echo "Building boost numeric bindings..."
tar -xvzf boost-numeric-bindings-20081116.tar.gz
cd boost-numeric-bindings
cp -R boost/numeric/bindings ${MAINDIR}/include/boost/numeric

cd ..

echo "Building activemq..."
tar -xvzf activemq-cpp-library-3.9.0-src.tar.gz
cd activemq-cpp-library-3.9.0
./configure --prefix=${MAINDIR} --enable-shared 
make -j6 install

cd ..

echo "Building ode..."
tar -xvzf ode-0.12.tar.gz
cd ode-0.12
./configure --prefix=${MAINDIR} --enable-shared 
make -j6 install

cd ..

echo "Building google protocol buffers..."
tar -xvzf protobuf-2.5.0.tar.gz
cd protobuf-2.5.0
./configure --prefix=${MAINDIR} --enable-shared 
make -j6 install

cd ..


echo "Finished building linux dependencies for SmartBody"

