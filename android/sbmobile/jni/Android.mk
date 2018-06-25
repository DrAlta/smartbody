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

LOCAL_SHORT_COMMANDS := true
SBM_ANDROID_LOCAL_PATH := $(call my-dir)
SBM_PATH := ../../../core/smartbody/SmartBody/
#include $(CLEAR_VARS)
include $(SBM_ANDROID_LOCAL_PATH)/../../smartbody/jni/Android.mk

include $(SBM_ANDROID_LOCAL_PATH)/../../alib/jni/Android.mk

#include $(CLEAR_VARS)
#LOCAL_MODULE := libalib_prebuilt
#LOCAL_SRC_FILES := ../../lib/libalib.a
#include $(PREBUILT_STATIC_LIBRARY)

#include $(CLEAR_VARS)
#LOCAL_MODULE := libjs_prebuilt
#LOCAL_SRC_FILES := ../../lib/libjs.so
#include $(PREBUILT_SHARED_LIBRARY)

#ALIB_LIBS :=  libxml_prebuilt icuuc_prebuilt libnlopt_prebuilt  libxslt_prebuilt boost-thread-prebuilt boost-system-prebuilt libalib_prebuilt

LOCAL_PATH = $(SBM_ANDROID_LOCAL_PATH)
include $(CLEAR_VARS)
SB_LIB_PATH := ../../../lib
LOCAL_MODULE    := libsbmobile
LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SBM_PATH)/../ode/include \
					$(LOCAL_PATH)/../../alib \
					$(LOCAL_PATH)/../../pythonLib_27/include/python2.7 \
					$(LOCAL_PATH)/../../boost \
					$(LOCAL_PATH)/../../boost_1_59/include \
					$(LOCAL_PATH)/$(SB_LIB_PATH)/bonebus/include \
					$(LOCAL_PATH)/$(SB_LIB_PATH)/vhcl/include \
					$(LOCAL_PATH)/$(SB_LIB_PATH)/vhmsg/vhmsg-c/include \
					$(LOCAL_PATH)/$(SB_LIB_PATH)/wsp/wsp/include \
					$(LOCAL_PATH)/$(SB_LIB_PATH)/boostnumeric \
					$(LOCAL_PATH)/$(CEREVOICE_LIB_DIR)/cerevoice_eng/include \
					$(LOCAL_PATH)/$(SBM_PATH)/../steersuite-1.3/external/ \
					$(LOCAL_PATH)/$(SBM_PATH)/../steersuite-1.3/steerlib/include \
					$(LOCAL_PATH)/$(SBM_PATH)/../steersuite-1.3/pprAI/include \
					$(LOCAL_PATH)/$(SBM_PATH)/../../../android/include \
					$(LOCAL_PATH)/$(SBM_PATH)/src 
					
LOCAL_CFLAGS    := -O3 -DBUILD_ANDROID -frtti -fexceptions
#LOCAL_CFLAGS    := -O3 -DBUILD_ANDROID -frtti -fexceptions -DUSE_CEREVOICE
LOCAL_SRC_FILES :=  SBMain.cpp Shader.cpp esUtil.c AppListener.cpp SBWrapper.cpp SBMobile.cpp
LOCAL_LDLIBS    := -landroid -llog -lGLESv3
LOCAL_STATIC_LIBRARIES := smartbody  xerces-prebuilt boost-filesystem-prebuilt boost-system-prebuilt boost-regex-prebuilt boost-python-prebuilt lapack blas f2c vhcl vhmsg bonebus iconv-prebuilt pprAI steerlib ann ode activemq-prebuilt apr-prebuilt apr-util-prebuilt expat-prebuilt libjerome openal alut tremolo sndfile $(CEREVOICE_LIBS) $(ALIB_LIBS)
LOCAL_SHARED_LIBRARIES := python-prebuilt-share libjs_prebuilt

include $(BUILD_SHARED_LIBRARY) 
