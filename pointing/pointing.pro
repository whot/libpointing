# pointing/pointing.pro --
#
# Initial software
# Authors: Nicolas Roussel
# Copyright © INRIA

TEMPLATE = lib

CONFIG += staticlib create_prl
CONFIG -= qt
#CONFIG += shared

TARGET   = pointing

INCLUDEPATH += ..
VPATH += ..

include(pointing-common.pri)

HERE = $$PWD
    
HEADERS  = $$HERE/pointing.h \
           $$HERE/utils/Base64.h \
           $$HERE/utils/ByteOrder.h \
           $$HERE/utils/Correlation.h \
           $$HERE/utils/FileUtils.h \
           $$HERE/utils/TimeStamp.h \
           $$HERE/utils/URI.h \
           $$HERE/utils/ConfigDict.h \
           $$HERE/utils/HIDTags.h \
           $$HERE/utils/HIDItem.h \
           $$HERE/utils/HIDReportParser.h \
           $$HERE/utils/PointingCursor.h \
           $$HERE/input/DummyPointingDevice.h \
           $$HERE/input/PointingDevice.h \
           $$HERE/input/SystemPointingDevice.h \
           $$HERE/output/DisplayDevice.h \
           $$HERE/output/DummyDisplayDevice.h \
           $$HERE/transferfunctions/Composition.h \
           $$HERE/transferfunctions/ConstantFunction.h \
           $$HERE/transferfunctions/NaiveConstantFunction.h \
           $$HERE/transferfunctions/SigmoidFunction.h \
           $$HERE/transferfunctions/TransferFunction.h \
           $$HERE/input/PointingDeviceManager.h \
           $$HERE/output/DisplayDeviceManager.h \
           $$HERE/transferfunctions/Interpolation.h \
           $$HERE/transferfunctions/SubPixelFunction.h

SOURCES  = $$HERE/utils/Base64.cpp \
           $$HERE/utils/ByteOrder.cpp \
           $$HERE/utils/FileUtils.cpp \
           $$HERE/utils/TimeStamp.cpp \
           $$HERE/utils/URI.cpp \
           $$HERE/utils/ConfigDict.cpp \
           $$HERE/utils/HIDItem.cpp \
           $$HERE/utils/HIDReportParser.cpp \
           $$HERE/utils/PointingCursor.cpp \
           $$HERE/input/PointingDevice.cpp \
           $$HERE/input/SystemPointingDevice.cpp \
           $$HERE/input/DummyPointingDevice.cpp \
           $$HERE/output/DisplayDevice.cpp \
           $$HERE/output/DummyDisplayDevice.cpp \
           $$HERE/transferfunctions/Composition.cpp \
           $$HERE/transferfunctions/ConstantFunction.cpp \
           $$HERE/transferfunctions/NaiveConstantFunction.cpp \
           $$HERE/transferfunctions/SigmoidFunction.cpp \
           $$HERE/transferfunctions/TransferFunction.cpp \
           $$HERE/input/PointingDeviceManager.cpp \
           $$HERE/output/DisplayDeviceManager.cpp \
           $$HERE/transferfunctions/Interpolation.cpp \
           $$HERE/transferfunctions/SubPixelFunction.cpp

include(../pointing-xorg/pointing-xorg.pri)

macx {
  HEADERS += $$HERE/utils/osx/osxTimeUtils.h \
             $$HERE/utils/osx/osxPlistUtils.h \
             $$HERE/input/osx/osxPointingDevice.h \
             $$HERE/input/osx/osxHIDUtils.h \
             $$HERE/input/osx/osxPrivateMultitouchDevice.h \
             $$HERE/input/osx/osxPrivateMultitouchSupport.h \
             $$HERE/input/osx/osxPointingDeviceManager.h \
             $$HERE/output/osx/osxDisplayDevice.h \
             $$HERE/output/osx/osxDisplayDeviceManager.h \
             $$HERE/transferfunctions/osx/osxSystemPointerAcceleration.h
  SOURCES += $$HERE/utils/osx/osxPlistUtils.cpp \
             $$HERE/input/osx/osxPointingDevice.cpp \
             $$HERE/input/osx/osxHIDUtils.cpp \
             $$HERE/input/osx/osxPrivateMultitouchDevice.cpp \
             $$HERE/input/osx/osxPointingDeviceManager.cpp \
             $$HERE/output/osx/osxDisplayDevice.cpp \
             $$HERE/output/osx/osxDisplayDeviceManager.cpp \
             $$HERE/transferfunctions/osx/osxSystemPointerAcceleration.cpp
  LIBS    += -F/System/Library/PrivateFrameworks -framework MultitouchSupport \
             -framework IOKit -framework CoreFoundation -framework ApplicationServices -framework AppKit
}

unix:!macx {
  HEADERS += $$HERE/input/linux/linuxPointingDevice.h \
             $$HERE/output/linux/xorgDisplayDevice.h \
             $$HERE/transferfunctions/linux/xorgSystemPointerAcceleration.h \
             $$HERE/input/linux/linuxPointingDeviceManager.h \
             $$HERE/output/linux/xorgDisplayDeviceManager.h
  SOURCES += $$HERE/input/linux/linuxPointingDevice.cpp \
             $$HERE/output/linux/xorgDisplayDevice.cpp \
             $$HERE/transferfunctions/linux/xorgSystemPointerAcceleration.cpp \
             $$HERE/input/linux/linuxPointingDeviceManager.cpp \
             $$HERE/input/linux/XInputHelper.cpp \
             $$HERE/output/linux/xorgDisplayDeviceManager.cpp
  LIBS    += -ludev -lpthread -lXrandr -lX11 -ldl -lXi
}

windows {
  # See http://blogs.msdn.com/b/oldnewthing/archive/2007/04/11/2079137.aspx
  #     and http://msdn.microsoft.com/en-us/library/aa383745.aspx
  DEFINES += _WIN32_WINNT="0x0600"      # Windows Vista
  DEFINES += _WIN32_WINDOWS="0x0600"    # ???
  DEFINES += NTDDI_VERSION="0x06000000" # Windows Vista

win32-msvc* {
  QMAKE_LFLAGS_DEBUG += /INCREMENTAL:NO
  QMAKE_CXXFLAGS_WARN_ON += -wd 4244 -wd 4305 -wd 4800 -wd 4312 -wd 4267
  QMAKE_CXXFLAGS += -D _CRT_SECURE_NO_WARNINGS
}
  CONFIG  += windows console
  HEADERS += $$HERE/utils/windows/winGetTimeOfDay.h \
             $$HERE/input/windows/USB.h \
             $$HERE/input/windows/winPointingDevice.h \
             $$HERE/output/windows/winDisplayDevice.h \
             $$HERE/output/windows/winDisplayDeviceHelper.h \
             $$HERE/output/windows/winDisplayDeviceManager.h \
             $$HERE/transferfunctions/windows/winSystemPointerAcceleration.h \
             $$HERE/input/windows/winPointingDeviceManager.h

  SOURCES += $$HERE/utils/windows/winGetTimeOfDay.cpp \
             $$HERE/input/windows/USB.cpp \
             $$HERE/input/windows/winPointingDevice.cpp \
             $$HERE/output/windows/winDisplayDevice.cpp \
             $$HERE/output/windows/winDisplayDeviceHelper.cpp \
             $$HERE/output/windows/winDisplayDeviceManager.cpp \
             $$HERE/transferfunctions/windows/winSystemPointerAcceleration.cpp \
             $$HERE/input/windows/winPointingDeviceManager.cpp
# TODO : Check those libraries:
  LIBS    += -L$$HERE/libs/windows -ldinput8 -ldxguid -lsetupapi -lgdi32 -lwbemuuid  -lAdvapi32 -luser32 -lHid# -lcomsupp
}

OTHER_FILES += \
    pointing.pri
