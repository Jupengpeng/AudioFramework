##APP_CPPFLAGS += -frtti
##APP_STL := stlport_static
#APP_ABI := armeabi #armeabi-v7a
##APP_OPTIM := debug


#add by binchen
#APP_STL := gnustl_static
#APP_CPPFLAGS += -frtti
APP_STL := stlport_static
APP_ABI := armeabi armeabi-v7a
APP_OPTIM := release
#APP_OPTIM := debug


include $(CLEAR_VARS)

APP_PLATFORM := android-9
APP_CFLAGS += -DANDROID_NDK -D_RELEASE -DSTDC_HEADERS
APP_ABI := armeabi armeabi-v7a
APP_CPPFLAGS += -frtti

APP_STL := stlport_static
APP_OPTIM := release
LOCAL_MODULE    := mp3lamess

LOCAL_LDFLAGS := -Wl,--build-id,-Wno-cpp
LOCAL_LDLIBS := \
	-lm \
	-llog \
	-ljnigraphics \