# apps/glutExample/glutExample.pro --
#
# Initial software
# Authors: Gery Casiez, Nicolas Roussel
# Copyright © Inria

TEMPLATE = app
CONFIG   += warn_on link_prl

POINTING = ../..
include($$POINTING/pointing/pointing.pri)

SOURCES += glutExample.cpp

macx {
  LIBS += -framework GLUT -framework OpenGL
}

unix:!macx {
  LIBS += -lglut -lGL
}

windows {
  LIBS += -lglut32 -lopengl32
}
