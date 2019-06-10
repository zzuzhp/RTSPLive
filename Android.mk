LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    XUtil/XUFunctorCommandTask.cpp \
    XUtil/XULog.cpp \
    XUtil/XUMutex.cpp \
    XUtil/XURefCount.cpp \
    XUtil/XUStat.cpp \
    XUtil/XUThread.cpp \
    XUtil/XUTime.cpp \
    UTE/UTE.cpp \
    UTE/UTEAcceptor.cpp \
    UTE/UTECore.cpp \
    UTE/UTETcpTransport.cpp \
    UTE/UTEUdpTransport.cpp \
    RTSPLive/AACStream.cpp \
    RTSPLive/AVCStream.cpp \
    RTSPLive/AVStream.cpp \
    RTSPLive/RtcpPacket.cpp \
    RTSPLive/RtpPacket.cpp \
    RTSPLive/RTSPClient.cpp \
    RTSPLive/RTSPCore.cpp \
    RTSPLive/RTSPLive.cpp \
    RTSPLive/RTSPMessager.cpp \
    RTSPLive/SDPEncoder.cpp

LOCAL_C_INCLUDES += $(LOCAL_PATH)/UTE
LOCAL_CFLAGS += -O3
LOCAL_MODULE := librtsplive

include $(BUILD_STATIC_LIBRARY)
