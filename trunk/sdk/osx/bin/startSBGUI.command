cd "$(dirname "$0")"
export DYLD_LIBRARY_PATH=/System/Library/Frameworks/Python.framework
export DYLD_LIBRARY_PATH=../lib:$DYLD_LIBRARY_PATH
./sbgui
