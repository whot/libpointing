# bindings/Java/Jar/pointing.pro --
#
# Initial software
# Authors: Géry Casiez
# Copyright © INRIA

TEMPLATE = lib
CONFIG   +=  warn_on dynamiclib
CONFIG   -=  qt

DESTDIR = ../build
TARGET   = pointing
INCLUDEPATH += ../../../../

# QT += xml

unix {
  HERE = $(PWD)/../..
} else {
  HERE = $$PWD/..
}

QMAKE_CXXFLAGS += -std=c++11

HEADERS  = $$HERE/native/org_libpointing_DisplayDevice.h \
           $$HERE/native/org_libpointing_PointingDevice.h \
           $$HERE/native/org_libpointing_PointingDeviceManager.h \
           $$HERE/native/org_libpointing_DisplayDeviceManager.h \
           $$HERE/native/org_libpointing_TransferFunction.h \
           $$HERE/native/org_libpointing_TimeStamp.h \
           $$HERE/native/handle.h


SOURCES  = $$HERE/native/DisplayDevice.cpp \
           $$HERE/native/PointingDevice.cpp \
           $$HERE/native/PointingDeviceManager.cpp \
           $$HERE/native/DisplayDeviceManager.cpp \
           $$HERE/native/TransferFunction.cpp \
           $$HERE/native/TimeStamp.cpp

macx {
    INCLUDEPATH += /System/Library/Frameworks/JavaVM.framework/Headers

    LIBS += -framework JavaVM -L../../../../pointing -lpointing -F/System/Library/PrivateFrameworks -framework MultitouchSupport -framework IOKit -framework CoreFoundation -framework ApplicationServices -framework AppKit -dynamiclib
}

# Use project in the folder ../msvc instead of this to compile bindings
windows {
    CONFIG += windows
    INCLUDEPATH += $$quote(C:\Program Files\Java\jdk1.6.0_27\include)
    INCLUDEPATH += $$quote(C:\Program Files\Java\jdk1.6.0_27\include\win32)
    LIBS    += -L$$quote(C:\Qt\2010.05\mingw\lib)  -lsetupapi -lgdi32 -L$$HERE/pro -lpointing -ldinput8 -ldxguid
}

unix {
    INCLUDEPATH += $$quote($(JAVA_HOME)/include/)
    INCLUDEPATH += $$quote($(JAVA_HOME)/include/linux)
    LIBS += -L../../../../pointing -lpointing
}
