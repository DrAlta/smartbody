
PROJECT:=smartbody-dll
PROJECT_DIR=../../../core/smartbody/$(PROJECT)
INCLUDE=-I$(PROJECT_DIR) -I$(PROJECT_DIR)/include -I../../../lib/vhcl/include -I$(SRC_DIR) -I../../../lib/vhmsg/vhmsg-c/include -I../../xerces-c/src -I../../../lib/boost -I$(PROJECT_DIR)/../steersuite-1.3/steerlib/include \
-I$(PROJECT_DIR)/../steersuite-1.3/pprAI/include -I$(PROJECT_DIR)/../steersuite-1.3/external -I../../../lib/bonebus/include -I../../../lib/wsp/wsp/include -I../../../lib/boost -I../../../lib/ -I../../../core/smartbody/smartbody-lib/src -I../../ode/include

WARNINGS:=-Wno-long-long -Wall
CXX_FLAGS:=-g -O0 -pthread -std=gnu++98 $(WARNINGS)
LD_FLAGS:=../../bonebus/lib32/bonebus.a \
../../pprAI/lib32/pprAI.a \
../smartbody-lib/lib32/smartbody-lib.a \
../../steerlib/lib32/steerlib.a \
../../tinyxml/lib32/tinyxml.a \
../../vhcl/lib32/vhcl.a \
../../vhmsg/lib32/vhmsg.a \
../../wsp/lib32/wsp.a \
../../xerces-c/src/.libs/libxerces-c.a \
../../ode/ode/src/.libs/libode.a

SRC_DIR=$(PROJECT_DIR)
OBJ_DIR=obj
LIB32_DIR=lib32
LIB64_DIR=lib64
SRC:=$(wildcard $(SRC_DIR)/*.cpp)
OBJS_x86_32:=$(patsubst %.cpp, obj/%_x86_32.o, $(notdir $(SRC)))
OBJS_x86_64:=$(patsubst %.cpp, obj/%_x86_64.o, $(notdir $(SRC)))

LIB_x86_32:=$(LIB32_DIR)/$(PROJECT).so
LIB_x86_64:=$(LIB64_DIR)/$(PROJECT).so

OS_NAME:=$(shell python $(NACL_SDK_ROOT)/tools/getos.py)
TC_PATH:=$(abspath $(NACL_SDK_ROOT)/toolchain/$(OS_NAME)_x86_newlib)
CXX:=$(TC_PATH)/bin/i686-nacl-gcc
AR:=$(TC_PATH)/bin/i686-nacl-ar

COPY:= python $(NACL_SDK_ROOT)/tools/oshelpers.py cp
MKDIR:= python $(NACL_SDK_ROOT)/tools/oshelpers.py mkdir
RM:= python $(NACL_SDK_ROOT)/tools/oshelpers.py rm
MV:= python $(NACL_SDK_ROOT)/tools/oshelpers.py mv

CYGWIN ?= nodosfilewarning
export CYGWIN 

all: lib32/smartbody-dll.so
	$(foreach(obj, $(OBJS_x86_32), $(MV) obj ./*)
lib32:
	$(MKDIR) -p $(LIB32_DIR)

lib64:
	$(MKDIR) -p $(LIB64_DIR)

obj:
	$(MKDIR) -p $(OBJ_DIR)
	
lib32/smartbody-dll.so: obj/smartbody-dll.o obj/smartbody-c-dll.o | lib32
	$(CXX)  -o $@  obj/smartbody-c-dll.o obj/smartbody-dll.o -m32 $(CXX_FLAGS) $(LD_FLAGS) -shared
	
#LIB_x86_64: obj/smartbody-dll.o obj/smartbody-c-dll.o | lib64
#	$(CXX) -o $@ $< -m64 $(CXX_FLAGS) $(LD_FLAGS) -shared
	
obj/smartbody-dll.o: $(SRC_DIR)/smartbody-dll.cpp | obj
	$(CXX) -o $@ -c $(INCLUDE) $< -m32 $(CXX_FLAGS) -fPIC

obj/smartbody-c-dll.o: $(SRC_DIR)/smartbody-c-dll.cpp | obj
	$(CXX) -o $@ -c $(INCLUDE) $< -m32 $(CXX_FLAGS) -fPIC
	
.PHONY: clean
clean :
		$(RM) -f $(OBJ_DIR)/*.o
		$(RM) -f $(LIB_x86_32)
		$(RM) -f $(LIB_x86_64)
		
		