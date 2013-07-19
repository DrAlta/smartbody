set(IPHONE_SDK "6.1")
set(IPHONE_ROOT "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer")
set(IPHONE_SDK_ROOT "${IPHONE_ROOT}/SDKs/iPhoneOS${IPHONE_SDK}.sdk")

set(CMAKE_FIND_ROOT_PATH "${IPHONE_SDK_ROOT}" "${IPHONE_ROOT}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_SYSTEM_NAME "GNU")
set(CMAKE_SYSTEM_PROCESSOR armv7)
set(CMAKE_OSX_ARCHITECTURES armv7)

set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)

set(CMAKE_C_COMPILER llvm-gcc-4.2)
set(CMAKE_CXX_COMPILER llvm-g++-4.2)

add_definitions("-D__IPHONE__")
add_definitions("-arch armv7 -pipe -no-cpp-precomp --sysroot=${IPHONE_SDK_ROOT} -miphoneos-version-min=${IPHONE_SDK}")
set(CMAKE_C_LINK_FLAGS "-arch armv7 --isysroot=${IPHONE_SDK_ROOT} -miphoneos-version-min=${IPHONE_SDK} -L${IPHONE_SDK_ROOT}/usr/lib -L${IPHONE_SDK_ROOT}/usr/lib/system")
set(CMAKE_CXX_LINK_FLAGS ${CMAKE_C_LINK_FLAGS})

include_directories("${IPHONE_SDK_ROOT}/usr/include")
include_directories("${IPHONE_SDK_ROOT}/usr/include/c++/4.2.1")
include_directories("${IPHONE_SDK_ROOT}/usr/include/c++/4.2.1/armv7-apple-darwin10")

link_directories("${IPHONE_SDK_ROOT}/usr/lib")
link_directories("${IPHONE_SDK_ROOT}/usr/lib/system")

set(CMAKE_CROSSCOMPILING TRUE)
set(IPHONE TRUE)
