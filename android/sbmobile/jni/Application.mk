NDK_TOOLCHAIN_VERSION := 4.9
APP_STL := gnustl_static
#APP_STL := gnustl_shared
APP_ABI := armeabi-v7a
#APP_ABI := armeabi
APP_PLATFORM := android-19
APP_MODULES := sbmobile
APP_OPTIM := release
#APP_CPPFLAGS := -O3 -std=c++11 -fexceptions -frtti -DBOOST_NO_CXX11_SCOPED_ENUMS -DSB_NO_ASSIMP
APP_CPPFLAGS := -O3 -std=c++11 -fexceptions -fpermissive -frtti -DBOOST_NO_CXX11_SCOPED_ENUMS -DBOOST_RESULT_OF_USE_DECLTYPE -DNDEBUG -DSB_NO_ASSIMP
#APP_CPPFLAGS := -O0 -g -std=c++11 -fexceptions -frtti -DBOOST_NO_CXX11_SCOPED_ENUMS -DBOOST_RESULT_OF_USE_DECLTYPE

APP_SHORT_COMMANDS := true