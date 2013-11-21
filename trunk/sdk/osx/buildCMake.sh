# make using standard cmake
mkdir -p buildCMake
cd buildCMake
cmake ..
make -j 6
sudo make install
cd ..

