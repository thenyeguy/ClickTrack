# NDK boilerplate
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# Set compiler flags
#LOCAL_CFLAGS += -std=c++11 -Wall -Wno-delete-non-virtual-dtor -Werror -g
LOCAL_CPPFLAGS := -std=c++11 -pthread -frtti -fexceptions -g \
	-Wall -Werror -Wno-sign-compare -Wno-maybe-uninitialized

# Define our target and set the source files to compile
LOCAL_MODULE := clicktrack
LOCAL_SRC_FILES := $(wildcard $(LOCAL_PATH)/src/*.cpp)

include $(BUILD_SHARED_LIBRARY)
