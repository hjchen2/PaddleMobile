LOCAL_PATH := $(call my-dir)

# change this folder path to yours
PADDLE_INSTALL_PATH := /Users/baidu/Project/paddle/build_android/install

include $(CLEAR_VARS)
LOCAL_MODULE := paddle_capi_whole
LOCAL_SRC_FILES := $(PADDLE_INSTALL_PATH)/lib/armeabi-v7a/libpaddle_capi_whole.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := glog
LOCAL_SRC_FILES := $(PADDLE_INSTALL_PATH)/third_party/glog/lib/armeabi-v7a/libglog.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := gflags
LOCAL_SRC_FILES := $(PADDLE_INSTALL_PATH)/third_party/gflags/lib/armeabi-v7a/libgflags.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := protobuf
LOCAL_SRC_FILES := $(PADDLE_INSTALL_PATH)/third_party/protobuf/lib/armeabi-v7a/libprotobuf.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := mobilenet
LOCAL_SRC_FILES := mobilenet_jni.cpp

LOCAL_C_INCLUDES := $(PADDLE_INSTALL_PATH)/include

LOCAL_WHOLE_STATIC_LIBRARIES := paddle_capi_whole
LOCAL_STATIC_LIBRARIES := glog gflags protobuf

LOCAL_CFLAGS := -O2 -fvisibility=hidden -fomit-frame-pointer -fstrict-aliasing -ffunction-sections -fdata-sections -ffast-math
LOCAL_CPPFLAGS := -O2 -fvisibility=hidden -fvisibility-inlines-hidden -fomit-frame-pointer -fstrict-aliasing -ffunction-sections -fdata-sections -ffast-math
#LOCAL_CFLAGS := -O2
#LOCAL_CPPFLAGS := -O2
LOCAL_LDFLAGS += -Wl,--gc-sections

LOCAL_CFLAGS += -fopenmp
LOCAL_CPPFLAGS += -fopenmp
LOCAL_LDFLAGS += -fopenmp

LOCAL_LDLIBS := -lz -llog -ljnigraphics

LOCAL_ALLOW_UNDEFINED_SYMBOLS := true

include $(BUILD_SHARED_LIBRARY)
