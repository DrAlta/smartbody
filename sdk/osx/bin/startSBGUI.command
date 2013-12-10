cd "$(dirname "$0")"
DYLD_LIBRARY_PATH=../lib:$DYLD_LIBRARY_PATH PYTHONHOME=../Python27 ./sbgui
