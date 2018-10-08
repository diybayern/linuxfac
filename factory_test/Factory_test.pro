#-------------------------------------------------
#
# Project created by QtCreator 2018-09-10T09:34:14
#
#-------------------------------------------------

QT       += core gui
CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = factory_test
TEMPLATE = app


SOURCES += main.cpp\
    core/func/BrightTest.cpp \
    core/func/CameraTest.cpp \
    core/func/EdidTest.cpp \
    core/func/FanTest.cpp \
    core/func/FuncBase.cpp \
    core/func/FuncTest.cpp \
    core/func/HddTest.cpp \
    core/func/MemTest.cpp \
    core/func/NetTest.cpp \
    core/func/NextProcess.cpp \
    core/func/SoundTest.cpp \
    core/func/StressTest.cpp \
    core/func/UploadMes.cpp \
    core/func/UsbTest.cpp \
    core/func/WifiTest.cpp \
    core/func/i2c.cpp \
    core/func/parse_edid.cpp \
    core/ui/UiHandle.cpp \
    utils/fac_log.cpp \
    utils/fac_utils.cpp \
    core/ui/DisplayTestWindow.cpp \
    core/ui/MainTestWindow.cpp \
    core/ui/StressTestWindow.cpp \
    core/ui/messageform.cpp \
    core/control/Control.cpp \
    core/ui/VideoTestThread.cpp \
    core/ui/ImageTestThread.cpp \
    core/ui/myeventloop.cpp	


HEADERS  += \
    inc/Control.h \
    inc/EdidTest.h \
    inc/fac_log.h \
    inc/fac_utils.h \
    inc/FuncBase.h \
    inc/FuncTest.h \
    inc/NetTest.h \
    inc/SoundTest.h \
    inc/ui.h \
    inc/UploadMes.h \
    inc/WifiTest.h \
    inc/BrightTest.h \
    inc/CameraTest.h \
    inc/UiHandle.h \
    inc/messageform.h \
    inc/VideoTestThread.h \
    inc/ImageTestThread.h \
    inc/i2c_dev.h \
    inc/libx86.h \
    inc/modes.h \

INCLUDEPATH += /usr/local/include/

#QMAKE_CFLAGS = `pkg-config --cflags --libs gstreamer-video-1.0 gstreamer-1.0 glib-2.0`

#QMAKE_CXXFLAGS = $$QMAKE_CFLAGS

#LIBS += -L/usr/lib/x86_64-linux-gnu  -lgobject-2.0 -lglib-2.0
#LIBS += `pkg-config --cflags --libs gstreamer-video-1.0 gstreamer-1.0 glib-2.0`

LIBS += -L/usr/local/include \
        -lavcodec \
        -lavdevice \
        -lavfilter \
        -lavformat \
        -lavutil \
        -lswresample \
        -lswscale

LIBS += -lx86 -lasound -ludev -lpthread -lavcodec -lavformat -lswscale -lavutil -lswresample -lva -lva-x11 -rdynamic -lm -lpng -lX11 -lftp
