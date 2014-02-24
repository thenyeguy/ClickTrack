# NDK boilerplate
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# Set compiler flags
LOCAL_CPPFLAGS := -std=c++11 -pthread -frtti -fexceptions -g \
	-Wall -Werror -Wno-sign-compare -Wno-maybe-uninitialized

# Define our source files 
LOCAL_LDLIBS += -lOpenSLES -llog
LOCAL_SRC_FILES := $(wildcard $(LOCAL_PATH)/src/*.cpp)

# Define our target lib
LOCAL_MODULE := clicktrack

include $(BUILD_SHARED_LIBRARY)
