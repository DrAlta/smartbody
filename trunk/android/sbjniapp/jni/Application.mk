#APP_STL := stlport_static
APP_STL := gnustl_static
APP_ABI := armeabi-v7a
NDK_TOOLCHAIN_VERSION := 4.8
APP_PLATFORM := android-18
APP_MODULES := sbjniapp
APP_OPTIM := release
APP_CPPFLAGS := -O3 -std=c++11 -fexceptions -fpermissive -frtti -DBOOST_NO_CXX11_SCOPED_ENUMS
