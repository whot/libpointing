# tests/pointingdevice/pointingdevice.pro --
#
# Initial software
# Authors: Izzat Mukhanov
# Copyright © INRIA

TEMPLATE  = app
CONFIG   += warn_on link_prl testcase
CONFIG   -= app_bundle

QT -= gui
QT += testlib

#QMAKE_CXXFLAGS += -mmacosx-version-min=10.7 -std=c++11 -stdlib=libc++
#LIBS += -mmacosx-version-min=10.7 -stdlib=libc++
#CONFIG += c++11

#QMAKE_CXXFLAGS += -mmacosx-version-min=10.7
#QMAKE_LFLAGS += -mmacosx-version-min=10.7
#QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7

TARGET = pointingdevice

POINTING = ../..
include($$POINTING/pointing/pointing.pri)

HEADERS   += pointingdevice.h
