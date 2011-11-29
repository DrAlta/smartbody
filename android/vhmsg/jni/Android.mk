# log4cxx
#
VHMSG_LOCAL_PATH := $(call my-dir)
LOCAL_PATH = $(VHMSG_LOCAL_PATH)

ANDROID_LIB_DIR := ../../lib
include $(CLEAR_VARS)
LOCAL_MODULE := activemq-prebuilt
LOCAL_SRC_FILES := $(ANDROID_LIB_DIR)/libactivemq-cpp.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := apr-prebuilt
LOCAL_SRC_FILES := $(ANDROID_LIB_DIR)/libapr.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := apr-util-prebuilt
LOCAL_SRC_FILES := $(ANDROID_LIB_DIR)/libaprutil-1.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := expat-prebuilt
LOCAL_SRC_FILES := $(ANDROID_LIB_DIR)/libexpat.a
include $(PREBUILT_STATIC_LIBRARY)

include $(LOCAL_PATH)/../../vhcl/jni/Android.mk

LOCAL_PATH = $(VHMSG_LOCAL_PATH)
include $(CLEAR_VARS)
LOCAL_MODULE := vhmsg
TARGET_PLATFORM := android-9
MY_VHMSG_DIR := ../vhmsg_src/
LOCAL_CFLAGS    := -DBUILD_ANDROID
LOCAL_C_INCLUDES := $(VHMSG_LOCAL_PATH)/$(MY_VHMSG_DIR) $(VHMSG_LOCAL_PATH)/$(MY_VHMSG_DIR)/../../vhcl/vhcl_src \
					$(VHMSG_LOCAL_PATH)/../../include/activemq-cpp/include
LOCAL_SRC_FILES := $(MY_VHMSG_DIR)/HttpUtility.cpp \
				   $(MY_VHMSG_DIR)/vhmsg-tt.cpp \
				   $(MY_VHMSG_DIR)/vhmsg.cpp				   

LOCAL_STATIC_LIBRARIES := vhcl activemq-prebuilt apr-prebuilt apr-util-prebuilt expat-prebuilt
include $(BUILD_STATIC_LIBRARY)


