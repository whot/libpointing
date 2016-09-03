# apps/consoleExample/consoleExample.pro --
#
# Initial software
# Authors: Gery Casiez
# Copyright © Inria

TEMPLATE  = app
CONFIG   += warn_on link_prl
CONFIG   -= app_bundle

TARGET = consoleExample

POINTING = ../..
include($$POINTING/pointing/pointing.pri)

HEADERS   +=
SOURCES   += consoleExample.cpp
