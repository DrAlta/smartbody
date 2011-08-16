# log4cxx
#

VHCL_LOCAL_PATH := $(call my-dir)
LOCAL_PATH = $(VHCL_LOCAL_PATH)
include $(CLEAR_VARS)
LOCAL_MODULE := vhcl
TARGET_PLATFORM := android-9
MY_VHCL_DIR := ../../../lib/vhcl/
LOCAL_CFLAGS    := -DBUILD_ANDROID
LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(MY_VHCL_DIR)/include $(LOCAL_PATH)/$(MY_VHCL_DIR)/log4cxx/src/main/include
LOCAL_SRC_FILES := $(MY_VHCL_DIR)/src/Pow2Assert.cpp \
				   $(MY_VHCL_DIR)/src/vhcl_audio.cpp \
				   $(MY_VHCL_DIR)/src/vhcl_log.cpp \
				   $(MY_VHCL_DIR)/src/vhcl_string.cpp \
				   $(MY_VHCL_DIR)/src/vhcl_timer.cpp \
				   $(MY_VHCL_DIR)/src/vhcl_memory.cpp \
				   $(MY_VHCL_DIR)/src/vhcl_crash.cpp 	

				   
include $(BUILD_STATIC_LIBRARY)
