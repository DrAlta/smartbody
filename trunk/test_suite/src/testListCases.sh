#!/bin/sh

cd $INPUTDIR
echo ""
echo "Test Case List:"
for file in $INPUTDIR/*
do
if [ -d "$file" ]; then
  echo "*  $file" 
fi
done