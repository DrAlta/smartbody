#!/bin/sh

cd $INPUTDIR
echo "Removing..."
while [ $# -gt 0 ]
do
  if [ -d "$1" ]; then
    rm -rf $1
    echo "*  $1 removed!"
  else
    echo "*  $1 not exist, skip"
  fi
  shift
done