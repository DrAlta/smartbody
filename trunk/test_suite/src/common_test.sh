#!/bin/sh

echo "Running test in directory: cd $SBMBIN" 
echo "Running test: ./$SBMEXE $SBMINPUT" 

cd $SBMBIN
./$SBMEXE $SBMINPUT
