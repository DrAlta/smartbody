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
ANDROID_LIB_DIR := ../../lib
#OGRE_DIR := ../../ogreSDK/
OGRE_DIR := ../../ogre/
#OGRE_LIB_DIR := $(OGRE_DIR)/Ogre/lib/armeabi-v7a/
OGRE_LIB_DIR := $(ANDROID_LIB_DIR)
#OGRE_DEP_LIB_DIR := $(OGRE_DIR)/Dependencies/lib/armeabi-v7a/
OGRE_DEP_LIB_DIR := $(ANDROID_LIB_DIR)
LOCAL_PATH = $(SBM_ANDROID_LOCAL_PATH)

include $(CLEAR_VARS)
LOCAL_MODULE := libogre
LOCAL_SRC_FILES := $(OGRE_LIB_DIR)/libOgreMainStatic.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := librts
#LOCAL_SRC_FILES := $(OGRE_LIB_DIR)/libOgreRTShaderSystemStatic.a
LOCAL_SRC_FILES := $(OGRE_LIB_DIR)/libOgreRTShaderSystem.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := plugin_particle
LOCAL_SRC_FILES := $(OGRE_LIB_DIR)/libPlugin_ParticleFXStatic.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := rs_gles2
LOCAL_SRC_FILES := $(OGRE_LIB_DIR)/libRenderSystem_GLES2Static.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libois
LOCAL_SRC_FILES := $(OGRE_DEP_LIB_DIR)/libOIS.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libzzip
#LOCAL_SRC_FILES := $(OGRE_DEP_LIB_DIR)/libzzip.a
LOCAL_SRC_FILES := $(OGRE_DEP_LIB_DIR)/libZZipLib.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libfreeimage
LOCAL_SRC_FILES := $(OGRE_DEP_LIB_DIR)/libFreeImage.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libfreetype
LOCAL_SRC_FILES := $(OGRE_DEP_LIB_DIR)/libfreetype.a
include $(PREBUILT_STATIC_LIBRARY)

include $(SBM_ANDROID_LOCAL_PATH)/../../smartbody/jni/Android.mk

LOCAL_PATH = $(SBM_ANDROID_LOCAL_PATH)
include $(CLEAR_VARS)
LOCAL_MODULE    := libsbogreapp
LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SBM_PATH)/../ode/include \
					$(LOCAL_PATH)/../../boost \
					$(LOCAL_PATH)/../../pythonLib/include/python2.6 \
					$(LOCAL_PATH)/$(SBM_PATH)/../../../lib/boost \
					$(LOCAL_PATH)/$(SBM_PATH)/../../../lib/bonebus/include \
					$(LOCAL_PATH)/$(SBM_PATH)/../../../lib/vhcl/include \
					$(LOCAL_PATH)/$(SBM_PATH)/../../../lib/vhmsg/vhmsg-c/include \
					$(LOCAL_PATH)/$(SBM_PATH)/../steersuite-1.3/external/ \
					$(LOCAL_PATH)/$(SBM_PATH)/../steersuite-1.3/steerlib/include \
					$(LOCAL_PATH)/$(SBM_PATH)/../steersuite-1.3/pprAI/include \
					$(LOCAL_PATH)/$(SBM_PATH)/../../../android/include \
					$(LOCAL_PATH)/$(SBM_PATH)/src \
					$(LOCAL_PATH)/$(OGRE_DIR)/Components/RTShaderSystem/include \
					$(LOCAL_PATH)/$(OGRE_DIR)/OgreMain/include \
					$(LOCAL_PATH)/$(OGRE_DIR)/RenderSystems/GLES2/include \
					$(LOCAL_PATH)/$(OGRE_DIR)/Dependencies/OIS/include
					#$(LOCAL_PATH)/$(OGRE_DIR)/Ogre/include/Components/RTShaderSystem \
					#$(LOCAL_PATH)/$(OGRE_DIR)/Ogre/include/Components/Overlay \
					#$(LOCAL_PATH)/$(OGRE_DIR)/Ogre/include/OgreMain \
					#$(LOCAL_PATH)/$(OGRE_DIR)/Ogre/include/Build \
					#$(LOCAL_PATH)/$(OGRE_DIR)/Ogre/include/RenderSystems/GLES2 \
					#$(LOCAL_PATH)/$(OGRE_DIR)/Dependencies/include/OIS
#LOCAL_CFLAGS    := -gstabs -g -DBUILD_ANDROID -frtti
LOCAL_CFLAGS    := -O3 -DBUILD_ANDROID -frtti -fexceptions
LOCAL_SRC_FILES := Main.cpp OgreDemoApp.cpp OgreFramework.cpp test.cpp SBListener.cpp AndroidLogListener.cpp 
LOCAL_LDLIBS    := -landroid -llog -lEGL -lGLESv2
#LOCAL_LDLIBS    := -llog -lEGL -lGLESv2 -gstabs
#LOCAL_SHARED_LIBRARIES := python-prebuilt 
#LOCAL_STATIC_LIBRARIES := rs_gles2 libogre cpufeatures  librts libois libfreeimage libfreetype libzzip smartbody xerces-prebuilt python-prebuilt boost-filesystem-prebuilt boost-system-prebuilt boost-regex-prebuilt boost-python-prebuilt lapack blas f2c vhcl vhmsg bonebus iconv-prebuilt pprAI steerlib ann ode activemq-prebuilt apr-prebuilt apr-util-prebuilt expat-prebuilt
LOCAL_STATIC_LIBRARIES := smartbody xerces-prebuilt python-prebuilt boost-filesystem-prebuilt boost-system-prebuilt boost-regex-prebuilt boost-python-prebuilt lapack blas f2c vhcl vhmsg bonebus iconv-prebuilt pprAI steerlib ann ode activemq-prebuilt apr-prebuilt apr-util-prebuilt expat-prebuilt libogre rs_gles2 cpufeatures librts libois libfreeimage libfreetype libzzip 
include $(BUILD_SHARED_LIBRARY) 
#$(call import-module,android/cpufeatures)
