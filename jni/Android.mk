LOCAL_PATH := $(call my-dir)
LOCAL_ENABLE_NEON := true

MY_CFLAGS := -D__STDC_LIMIT_MACROS -DPLATFORM_POSIX
MY_CFLAGS += -D__STDC_CONSTANT_MACROS -D_ANDROID_QC

MY_LOCAL_LIB_PATH := ${LOCAL_PATH}/../obj/local/armeabi-v7a

############################# Build coroutine Library

include $(CLEAR_VARS)

LOCAL_MODULE    := coroutine
LOCAL_SRC_FILES := src/coroutine.c src/switch-arm.S
LOCAL_C_INCLUDES += include
LOCAL_CFLAGS += $(MY_CFLAGS)
LOCAL_ARM_MODE := arm

include $(BUILD_STATIC_LIBRARY)

############################### Build pingpong executable

include $(CLEAR_VARS)

LOCAL_MODULE    := pingpong
LOCAL_SRC_FILES := src/pingpong.c
LOCAL_C_INCLUDES += include
LOCAL_CFLAGS += $(MY_CFLAGS)
LOCAL_LDFLAGS:= -L$(MY_LOCAL_LIB_PATH) -lcoroutine
LOCAL_STATIC_LIBRARIES := coroutine
LOCAL_ARM_MODE := arm

include $(BUILD_EXECUTABLE)

############################### Build nested executable

include $(CLEAR_VARS)

LOCAL_MODULE    := nested
LOCAL_SRC_FILES := src/nested.c
LOCAL_C_INCLUDES += include
LOCAL_CFLAGS += $(MY_CFLAGS)
LOCAL_LDFLAGS:= -L$(MY_LOCAL_LIB_PATH) -lcoroutine
LOCAL_STATIC_LIBRARIES := coroutine
LOCAL_ARM_MODE := arm

include $(BUILD_EXECUTABLE)

############################### Build test_poll executable

include $(CLEAR_VARS)

LOCAL_MODULE    := test_poll
LOCAL_SRC_FILES := src/test_poll.c
LOCAL_C_INCLUDES += include
LOCAL_CFLAGS += $(MY_CFLAGS) --std=c99
LOCAL_LDFLAGS:= -L$(MY_LOCAL_LIB_PATH) -lcoroutine
LOCAL_STATIC_LIBRARIES := coroutine
LOCAL_ARM_MODE := arm

include $(BUILD_EXECUTABLE)

############################### Build barrier executable

include $(CLEAR_VARS)

LOCAL_MODULE    := barrier
LOCAL_SRC_FILES := src/barrier.c
LOCAL_C_INCLUDES += include
LOCAL_CFLAGS += $(MY_CFLAGS)
LOCAL_LDFLAGS:= -L$(MY_LOCAL_LIB_PATH) -lcoroutine
LOCAL_STATIC_LIBRARIES := coroutine
LOCAL_ARM_MODE := arm

include $(BUILD_EXECUTABLE)

