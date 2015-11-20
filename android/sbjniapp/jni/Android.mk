# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
SBM_ANDROID_LOCAL_PATH := $(call my-dir)
SBM_PATH := ../../../core/smartbody/SmartBody/

include $(SBM_ANDROID_LOCAL_PATH)/../../smartbody/jni/Android.mk

LOCAL_PATH = $(SBM_ANDROID_LOCAL_PATH)
include $(CLEAR_VARS)
SB_LIB_PATH := ../../../lib
ANDROID_DIR := ../../
LOCAL_MODULE    := libsbjniapp
LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SBM_PATH)/../ode/include \
					$(LOCAL_PATH)/../../pythonLib_27/include/python2.7 \
					$(LOCAL_PATH)/../../boost \
					$(LOCAL_PATH)/$(BOOST_INCLUDE_DIR) \
					$(LOCAL_PATH)/$(SB_LIB_PATH)/festival/speech_tools/include \
					$(LOCAL_PATH)/$(SB_LIB_PATH)/festival/festival/src/include \
					$(LOCAL_PATH)/$(SB_LIB_PATH)/festival/festival/src/modules/VHDuration \
					$(LOCAL_PATH)/$(SB_LIB_PATH)/bonebus/include \
					$(LOCAL_PATH)/$(SB_LIB_PATH)/vhcl/include \
					$(LOCAL_PATH)/$(SB_LIB_PATH)/vhmsg/vhmsg-c/include \
					$(LOCAL_PATH)/$(SBM_PATH)/../steersuite-1.3/external/ \
					$(LOCAL_PATH)/$(SBM_PATH)/../steersuite-1.3/steerlib/include \
					$(LOCAL_PATH)/$(SBM_PATH)/../steersuite-1.3/pprAI/include \
					$(LOCAL_PATH)/$(SBM_PATH)/../../../android/include \
					$(LOCAL_PATH)/$(ANDROID_DIR)/gl-wes-v2/src \
					$(LOCAL_PATH)/$(SB_LIB_PATH)/assimp-3.1.1/include \
					$(LOCAL_PATH)/$(SBM_PATH)/src
LOCAL_CFLAGS    := -O3 -DBUILD_ANDROID -frtti -fexceptions -g
LOCAL_SRC_FILES := sbjniapp.cpp  Shader.cpp esUtil.c AppListener.cpp SBWrapper.cpp
LOCAL_LDLIBS    := -landroid -llog -lGLESv2
#LOCAL_SHARED_LIBRARIES := python-prebuilt 
LOCAL_STATIC_LIBRARIES := smartbody xerces-prebuilt boost-filesystem-prebuilt boost-system-prebuilt boost-regex-prebuilt python-prebuilt boost-python-prebuilt lapack blas f2c vhcl vhmsg bonebus iconv-prebuilt pprAI steerlib ann ode activemq-prebuilt apr-prebuilt apr-util-prebuilt expat-prebuilt 
#festival-prebuilt estools-prebuilt estbase-prebuilt eststring-prebuilt assimp gl-wes 
include $(BUILD_SHARED_LIBRARY) 
