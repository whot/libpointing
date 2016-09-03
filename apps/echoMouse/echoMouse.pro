# apps/echoMouse/echoMouse.pro --
#
# Initial software
# Authors: Izzat Mukhanov
# Copyright © Inria

TEMPLATE  = app
CONFIG   += warn_on link_prl
CONFIG   -= app_bundle
QT += gui

TARGET = echoMouse

POINTING = ../..
include($$POINTING/pointing/pointing.pri)

HEADERS += echoMouse.h
SOURCES += main.cpp echoMouse.cpp

unix {
    HEADERS += serial.h
    SOURCES += serial.cpp
}

linux {
    HEADERS += xorgEchoMouse.h
    SOURCES += xorgEchoMouse.cpp
}

macx {
    HEADERS   += osxEchoMouse.h
    SOURCES   += osxEchoMouse.cpp
}

windows {
    HEADERS  += hidapi.h winEchoMouse.h
    SOURCES  += hid.c winEchoMouse.cpp
}
