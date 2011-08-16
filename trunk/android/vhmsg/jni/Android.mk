# log4cxx
#
VHMSG_LOCAL_PATH := $(call my-dir)
LOCAL_PATH = $(VHMSG_LOCAL_PATH)
include $(LOCAL_PATH)/../../vhcl/jni/Android.mk

LOCAL_PATH = $(VHMSG_LOCAL_PATH)
include $(CLEAR_VARS)
LOCAL_MODULE := vhmsg
TARGET_PLATFORM := android-9
MY_VHMSG_DIR := ../../../lib/vhmsg/vhmsg-c/
LOCAL_CFLAGS    := -DBUILD_ANDROID
LOCAL_C_INCLUDES := $(VHMSG_LOCAL_PATH)/$(MY_VHMSG_DIR)/include $(VHMSG_LOCAL_PATH)/$(MY_VHMSG_DIR)/../../vhcl/include
LOCAL_SRC_FILES := $(MY_VHMSG_DIR)/src/HttpUtility.cpp \
				   $(MY_VHMSG_DIR)/src/vhmsg-tt.cpp \
				   $(MY_VHMSG_DIR)/src/vhmsg.cpp				   

LOCAL_STATIC_LIBRARIES := vhcl
include $(BUILD_STATIC_LIBRARY)


