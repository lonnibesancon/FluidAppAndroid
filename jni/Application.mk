# APP_ABI := armeabi armeabi-v7a
# APP_OPTIM := release

APP_ABI := armeabi-v7a
APP_OPTIM := debug

APP_PLATFORM := android-8
NDK_TOOLCHAIN_VERSION=4.8
# NDK_TOOLCHAIN_VERSION=4.7

# system stlport_static stlport_shared gnustl_static gnustl_shared gabi++_static gabi++_shared none
# APP_STL := gnustl_static
APP_STL := gnustl_shared
APP_CPPFLAGS += -frtti -fexceptions -std=c++11
