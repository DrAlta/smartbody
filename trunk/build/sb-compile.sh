#!/bin/sh


cd build/lib/festival/speech_tools
chmod 700 configure
./configure
make clean
make
make install
cd ../../../..

cd build/lib/festival/festival
chmod 700 configure
./configure
make clean
make
make install
cd ../../../..


mkdir build/bin
cd build/bin
cmake ..
make
make install
