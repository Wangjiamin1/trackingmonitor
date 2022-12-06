#-------------------------------------------------
#
# Project created by QtCreator 2022-09-04T19:13:14
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets  serialport

TARGET = trackingboard
TEMPLATE = app
QT += network



LIBS += -LD:/Program_Files/opencv-3.4.15/build/install/x64/mingw/lib -llibopencv_world3415
LIBS += -LD:/Program_Files/opencv-3.4.15/build/install/x64/mingw/bin -llibopencv_world3415

INCLUDEPATH += D:/Program_Files/opencv-3.4.15/build/install/include

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS




INCLUDEPATH += $$PWD/ffmpeg-5.0.1-full_build-shared/include

LIBS += -L$$PWD/ffmpeg-5.0.1-full_build-shared/lib -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale

LIBS += -L$$PWD/ffmpeg-5.0.1-full_build-shared/bin -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale



#LIBS += -LD:/18392/Documents/qt_project/trackingmonitor/ffmpeg-5.0.1-full_build-shared/lib -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale

CONFIG += c++11


INCLUDEPATH += include/

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/camprotocol.cpp \
    src/mythread.cpp \
    src/h264_decoder.cpp \
    src/mylabel.cpp

HEADERS += \
    include/mainwindow.h \
    include/camprotocol.h \
    include/mythread.h \
    include/h264_decoder.h \
    include/mylabel.h

FORMS += \
    ui/mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

