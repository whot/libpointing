# apps/transferFunctionEditor/transferFunctionEditor.pro --
#
# Initial software
# Authors: Izzatbek Mukhanov
# Copyright © Inria

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = transferFunctionEditor
TEMPLATE = app

POINTING = ../..
include($$POINTING/pointing/pointing.pri)

DEFINES += QCUSTOMPLOT_USE_LIBRARY

# Change the following 2 lines
INCLUDEPATH += ../../../qCustomPlot
LIBS += -lqcustomplot -L../../../qCustomPlot/debug

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
