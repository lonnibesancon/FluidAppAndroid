LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := artoolkit

LOCAL_SRC_FILES := $(notdir $(wildcard $(LOCAL_PATH)/*.c))

# Compiler flags
LOCAL_CFLAGS += -Ofast -funroll-loops

# Enable NEON instructions
LOCAL_ARM_NEON := true

# Use the more efficient 32-bit instructions, at the expense
# of a larger code size
LOCAL_ARM_MODE := arm

# # Logging library (__android_log_* functions)
# LOCAL_LDLIBS += -llog

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)

include $(BUILD_SHARED_LIBRARY)
