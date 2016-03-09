LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := native

# Recursive wildcard in pure make, no external commands required
# From: http://stackoverflow.com/a/12959764
rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

# Recursively find all *.cpp files under LOCAL_PATH
LOCAL_SRC_FILES := $(subst $(LOCAL_PATH)/,,$(call rwildcard,$(LOCAL_PATH),*.cpp))

# Compiler flags
# LOCAL_CFLAGS += -Ofast -Wall -Wextra -pedantic -Wno-unused-parameter -DNDEBUG
LOCAL_CFLAGS += -Ofast -Wall -Wextra -pedantic -Wno-unused-parameter -UNDEBUG
# LOCAL_CFLAGS += -O0 -g -Wall -Wextra -pedantic -Wno-unused-parameter

# (optional) Silence warnings in the NDK headers
LOCAL_CFLAGS += -isystem ${NDK_ROOT}/platforms/$(TARGET_PLATFORM)/arch-arm/usr/include

# Enable NEON instructions
LOCAL_ARM_NEON := true

# Use the more efficient 32-bit instructions, at the expense
# of a larger code size
LOCAL_ARM_MODE := arm

# Logging library (__android_log_* functions)
LOCAL_LDLIBS += -llog

# OpenGL ES 2.0
LOCAL_LDLIBS += -lGLESv2
# LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES

LOCAL_LDFLAGS += -L $(LOCAL_PATH)/../../thirdparty/vtk-android/lib/
LOCAL_LDFLAGS += -L $(LOCAL_PATH)/../../thirdparty/QCAR/libs/$(APP_ABI)/ -lQCAR

# NOTE: the link order is important!
LOCAL_LDFLAGS += -lvtkIOXML-6.0 -lvtkIOLegacy-6.0 -lvtkIOPLY-6.0 -lvtkIOGeometry-6.0 -lvtkFiltersModeling-6.0 -lvtkImagingCore-6.0 -lvtkRenderingFreeType-6.0 -lvtkRenderingCore-6.0 -lvtkIOImage-6.0 -lvtkDICOMParser-6.0 -lvtkmetaio-6.0 -lvtkpng-6.0 -lvtktiff-6.0 -lvtkjpeg-6.0 -lvtkFiltersSources-6.0 -lvtkFiltersGeometry-6.0 -lvtkIOXMLParser-6.0 -lvtkIOCore-6.0 -lvtkexpat-6.0 -lvtkFiltersExtraction-6.0 -lvtkFiltersGeneral-6.0 -lvtkFiltersCore-6.0 -lvtkCommonExecutionModel-6.0 -lvtkCommonComputationalGeometry-6.0 -lvtkCommonDataModel-6.0 -lvtkCommonMisc-6.0 -lvtkCommonTransforms-6.0 -lvtkCommonSystem-6.0 -lvtkCommonMath-6.0 -lvtkCommonCore-6.0 -lvtksys-6.0 -lvtkfreetype-6.0 -lvtkzlib-6.0

# Use -isystem to silence warnings in the VES headers
LOCAL_CFLAGS += -isystem $(LOCAL_PATH)/../../thirdparty/eigen
LOCAL_CFLAGS += -isystem $(LOCAL_PATH)/../../thirdparty/vtk-android/include/vtk-6.0

# LOCAL_C_INCLUDES += ${NDK_ROOT}/sources/cxx-stl/gnu-libstdc++/4.6/include
# LOCAL_C_INCLUDES += ${NDK_ROOT}/sources/cxx-stl/gnu-libstdc++/4.6/libs/armeabi-v7a/include

LOCAL_SHARED_LIBRARIES := artoolkit
# LOCAL_SHARED_LIBRARIES := artoolkitplus

include $(BUILD_SHARED_LIBRARY)
