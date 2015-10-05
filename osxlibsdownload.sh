#!/bin/sh

mkdir -p dependencies
echo "Downloading boost..."
curl http://iweb.dl.sourceforge.net/project/boost/boost/1.59.0/boost_1_59_0.tar.gz > dependencies/boost_1_59_0.tar.gz

echo "Downloading boost numeric bindings..."
curl http://mathema.tician.de/dl/software/boost-numeric-bindings/boost-numeric-bindings-20081116.tar.gz > dependencies/boost-numeric-bindings-20081116.tar.gz

echo "Downloading fltk..."
curl http://fltk.org/pub/fltk/1.3.3/fltk-1.3.3-source.tar.gz > dependencies/fltk-1.3.3-source.tar.gz

echo "Downloading activemq..."
curl http://mirror.cc.columbia.edu/pub/software/apache/activemq/activemq-cpp/3.9.0/activemq-cpp-library-3.9.0-src.tar.gz > dependencies/activemq-cpp-library-3.9.0-src.tar.gz

echo "Downloading ode..."
curl http://iweb.dl.sourceforge.net/project/opende/ODE/0.12/ode-0.12.tar.gz > dependencies/ode-0.12.tar.gz

echo "Downloading xerces..."
curl http://download.nextag.com/apache//xerces/c/3/sources/xerces-c-3.1.2.tar.gz > dependencies/xerces-c-3.1.2.tar.gz

echo "Downloading google protocol buffers..."
curl https://github.com/google/protobuf/releases/download/v2.5.0/protobuf-2.5.0.tar.gz > dependencies/protobuf-2.5.0.tar.gz

echo "Downloading glew..."
curl http://skylineservers.dl.sourceforge.net/project/glew/glew/1.6.0/glew-1.6.0.tgz > dependencies/glew-1.6.0.tgz

echo "Downloading freealut..."
curl http://distro.ibiblio.org/rootlinux/rootlinux-ports/more/freealut/freealut-1.1.0.tar.gz > dependencies/freealut-1.1.0.tar.gz

echo "Downloading libsndfile..."
curl http://www.mega-nerd.com/libsndfile/files/libsndfile-1.0.25.tar.gz > dependencies/libsndfile-1.0.25.tar.gz

echo "Finished downloading dependencies for SmartBody on OSX"
