#!/bin/sh

mkdir -p dependencies


echo "Downloading boost numeric bindings..."
curl http://mathema.tician.de/dl/software/boost-numeric-bindings/boost-numeric-bindings-20081116.tar.gz > dependencies/boost-numeric-bindings-20081116.tar.gz

echo "Downloading activemq..."
curl http://mirror.cc.columbia.edu/pub/software/apache/activemq/activemq-cpp/3.9.0/activemq-cpp-library-3.9.0-src.tar.gz > dependencies/activemq-cpp-library-3.9.0-src.tar.gz

echo "Downloading ode..."
curl http://iweb.dl.sourceforge.net/project/opende/ODE/0.12/ode-0.12.tar.gz > dependencies/ode-0.12.tar.gz

echo "Downloading google protocol buffers..."
curl http://smartbody.ict.usc.edu/dependencies/protobuf-2.5.0.tar.gz > dependencies/protobuf-2.5.0.tar.gz 

echo "Finished downloading dependencies for SmartBody on linux"
