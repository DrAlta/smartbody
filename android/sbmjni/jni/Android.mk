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
SBM_PATH := ../../../core/smartbody/smartbody-lib/

include $(SBM_ANDROID_LOCAL_PATH)/../../smartbody-lib/jni/Android.mk

LOCAL_PATH = $(SBM_ANDROID_LOCAL_PATH)
include $(CLEAR_VARS)
LOCAL_MODULE    := libsbmjni
LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SBM_PATH)/../ode/include \
					$(LOCAL_PATH)/$(SBM_PATH)/../../../lib/boost \
					$(LOCAL_PATH)/$(SBM_PATH)/../../../lib/bonebus/include \
					$(LOCAL_PATH)/$(SBM_PATH)/../../../lib/vhcl/include \
					$(LOCAL_PATH)/$(SBM_PATH)/../../../lib/vhmsg/vhmsg-c/include \
					$(LOCAL_PATH)/$(SBM_PATH)/../../../lib/wsp/wsp/include \
					$(LOCAL_PATH)/$(SBM_PATH)/../steersuite-1.3/external/ \
					$(LOCAL_PATH)/$(SBM_PATH)/../steersuite-1.3/steerlib/include \
					$(LOCAL_PATH)/$(SBM_PATH)/../steersuite-1.3/pprAI/include \
					$(LOCAL_PATH)/$(SBM_PATH)/../../../android/include \
					$(LOCAL_PATH)/$(SBM_PATH)/src
LOCAL_CFLAGS    := -g -O0 -DBUILD_ANDROID -frtti
LOCAL_SRC_FILES := sbmjni.cpp
LOCAL_LDLIBS    := -llog -lEGL -lGLESv1_CM
LOCAL_STATIC_LIBRARIES := sbm xerces-prebuilt boost-filesystem-prebuilt boost-system-prebuilt boost-regex-prebuilt boost-python-prebuilt lapack blas f2c vhcl wsp vhmsg bonebus iconv-prebuilt pprAI steerlib ann ode
LOCAL_SHARED_LIBRARIES := python-prebuilt
include $(BUILD_SHARED_LIBRARY) 