
PROJECT:=vhmsg
PROJECT_DIR=../../lib/$(PROJECT)/vhmsg-c
INCLUDE=-I$(PROJECT_DIR)/include -I../../lib/vhcl/include -I../../lib/activemq/activemq-cpp/src/main

WARNINGS:=-Wno-long-long -Wall
CXX_FLAGS:=-g -O0 -pthread -std=gnu++98 $(WARNINGS)
LD_FLAGS:=-g 

SRC_DIR=$(PROJECT_DIR)/src
OBJ_DIR=obj
LIB32_DIR=lib32
LIB64_DIR=lib64
SRC:=$(wildcard $(SRC_DIR)/*.cpp)
OBJS_x86_32:=$(patsubst %.cpp, obj/%_x86_32.o, $(notdir $(SRC)))
OBJS_x86_64:=$(patsubst %.cpp, obj/%_x86_64.o, $(notdir $(SRC)))

LIB_x86_32:=$(LIB32_DIR)/$(PROJECT).a
LIB_x86_64:=$(LIB64_DIR)/$(PROJECT).a

OS_NAME:=$(shell python $(NACL_SDK_ROOT)/tools/getos.py)
TC_PATH:=$(abspath $(NACL_SDK_ROOT)/toolchain/$(OS_NAME)_x86_glibc)
CXX:=$(TC_PATH)/bin/i686-nacl-g++
AR:=$(TC_PATH)/bin/i686-nacl-ar

COPY:= python $(NACL_SDK_ROOT)/tools/oshelpers.py cp
MKDIR:= python $(NACL_SDK_ROOT)/tools/oshelpers.py mkdir
RM:= python $(NACL_SDK_ROOT)/tools/oshelpers.py rm
MV:= python $(NACL_SDK_ROOT)/tools/oshelpers.py mv

CYGWIN ?= nodosfilewarning
export CYGWIN 

all: LIB_x86_32 LIB_x86_64
	$(foreach(obj, $(OBJS_x86_32), $(MV) obj ./*)
lib32:
	$(MKDIR) -p $(LIB32_DIR)

lib64:
	$(MKDIR) -p $(LIB64_DIR)

obj:
	$(MKDIR) -p $(OBJ_DIR)
	
LIB_x86_32: $(OBJS_x86_32) | lib32
	$(AR) rcs $(LIB_x86_32) $^
	
LIB_x86_64: $(OBJS_x86_64) | lib64
	$(AR) rcs $(LIB_x86_64) $^
	
$(OBJ_DIR)/%_x86_32.o: $(SRC_DIR)/%.cpp | obj
	$(CXX) -o $@ -c $(INCLUDE) $< -m32 $(CXXFLAGS) -fPIC

$(OBJ_DIR)/%_x86_64.o: $(SRC_DIR)/%.cpp | obj
	$(CXX) -o $@ -c $(INCLUDE) $< -m64 $(CXXFLAGS) -fPIC
	
.PHONY: clean
clean :
		$(RM) -f $(OBJ)/*.o
		$(RM) -f $(LIB_x86_32)
		$(RM) -f $(LIB_x86_64)
		
		