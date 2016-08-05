LOCAL_PATH := $(call my-dir)
TARGET_ARCH_ABI := armeabi-v7a


include $(CLEAR_VARS)
LOCAL_MODULE    := openssl
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/lib/libssl.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := opencrypto
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/lib/libcrypto.so
include $(PREBUILT_SHARED_LIBRARY)



include $(CLEAR_VARS)

LOCAL_C_INCLUDES += jni/$(TARGET_ARCH_ABI)/include
LOCAL_CFLAGS    := -std=c99 -O2 -W -Wall -pthread -pipe -DMG_ENABLE_SSL  $(COPT)
LOCAL_SRC_FILES :=  com_doordu_webserver_WebServer.c mongoose.c
LOCAL_MODULE := webserver
LOCAL_LDLIBS += -llog
LOCAL_SHARED_LIBRARIES := openssl opencrypto

include $(BUILD_SHARED_LIBRARY)
