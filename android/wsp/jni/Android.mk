# log4cxx
#
WSP_LOCAL_PATH := $(call my-dir)

#LOCAL_PATH = $(WSP_LOCAL_PATH)
#include $(LOCAL_PATH)/../../vhcl/jni/Android.mk

LOCAL_PATH = $(WSP_LOCAL_PATH)
include $(LOCAL_PATH)/../../vhmsg/jni/Android.mk

LOCAL_PATH := $(WSP_LOCAL_PATH)
include $(CLEAR_VARS)
LOCAL_MODULE := wsp
TARGET_PLATFORM := android-9
SB_LIB_DIR := ../../../lib
MY_WSP_DIR := ../../../lib/wsp/wsp/
LOCAL_CFLAGS    := -DBUILD_ANDROID
LOCAL_C_INCLUDES := $(WSP_LOCAL_PATH)/$(MY_WSP_DIR)/include $(WSP_LOCAL_PATH)/$(SB_LIB_DIR)/vhcl/include $(WSP_LOCAL_PATH)/$(SB_LIB_DIR)/vhmsg/vhmsg-c/include
LOCAL_SRC_FILES := $(MY_WSP_DIR)/src/wsp.cpp \
				   $(MY_WSP_DIR)/src/wsp_data_sources.cpp \
				   $(MY_WSP_DIR)/src/wsp_error.cpp \
				   $(MY_WSP_DIR)/src/wsp_impl.cpp \
				   $(MY_WSP_DIR)/src/wsp_subscriptions.cpp \
				   $(MY_WSP_DIR)/src/wsp_vector.cpp				   
LOCAL_STATIC_LIBRARIES := vhcl vhmsg
include $(BUILD_STATIC_LIBRARY)
#include $(BUILD_SHARED_LIBRARY)


