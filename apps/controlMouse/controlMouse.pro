# apps/controlMouse/controlMouse.pro --
#
# Initial software
# Authors: Izzat Mukhanov
# Copyright © Inria

TEMPLATE  = app
CONFIG   += warn_on link_prl
CONFIG   -= app_bundle

TARGET = controlMouse

POINTING = ../..
include($$POINTING/pointing/pointing.pri)

SOURCES   += controlMouse.cpp \
    FunctionChanger.cpp

HEADERS += \
    FunctionChanger.h
