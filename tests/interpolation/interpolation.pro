# tests/interpolation/interpolation.pro --
#
# Initial software
# Authors: Izzat Mukhanov
# Copyright © INRIA

TEMPLATE  = app
CONFIG   += warn_on link_prl
CONFIG   -= app_bundle

QT += testlib

TARGET = interpolation

POINTING = ../..
include($$POINTING/pointing/pointing.pri)

HEADERS   += interpolation.h
