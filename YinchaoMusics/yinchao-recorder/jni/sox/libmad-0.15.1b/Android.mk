LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)

LOCAL_MODULE := libmad
LOCAL_ARM_MODE := arm
LOCAL_C_INCLUDES := $(LOCAL_PATH) \

LOCAL_SRC_FILES := version.c fixed.c bit.c timer.c stream.c frame.c  \
                   synth.c decoder.c layer12.c layer3.c huffman.c  imdct_l_arm.S \

LOCAL_SHARED_LIBRARIES := liblpc10 libgsm
LOCAL_CFLAGS           := -Wall -g
LOCAL_LDFLAGS          := -Wl,-Map,xxx.map
LOCAL_LDLIBS := -ldl -lGLESv1_CM -llog -L$(DIRECTORY_TO_OBJ)

include $(BUILD_SHARED_LIBRARY)


