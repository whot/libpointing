# pointing/pointing.pri --
#
# Initial software
# Authors: Nicolas Roussel
# Copyright © INRIA

#message("Using pointing.pri")

QT += xml

CONFIG += link_prl

INCLUDEPATH += $$POINTING
DEPENDPATH  += $$POINTING/pointing

include(pointing-common.pri)

macx {
  LIBS += -L$$POINTING/pointing -lpointing
  POST_TARGETDEPS += $$POINTING/pointing/libpointing.a
}

unix:!macx {
  LIBS += -L$$POINTING/pointing -lpointing
  POST_TARGETDEPS += $$POINTING/pointing/libpointing.a
}

windows {
  # See http://blogs.msdn.com/b/oldnewthing/archive/2007/04/11/2079137.aspx
  #     and http://msdn.microsoft.com/en-us/library/aa383745.aspx
  DEFINES += _WIN32_WINNT="0x0600"      # Windows Vista
  DEFINES += _WIN32_WINDOWS="0x0600"    # ???
  DEFINES += NTDDI_VERSION="0x06000000" # Windows Vista

  win32-msvc* {
    QMAKE_LFLAGS_DEBUG += /INCREMENTAL:NO
    CONFIG(debug, debug|release){
      LIBS += -L$$POINTING/pointing/debug -lpointing
      PRE_TARGETDEPS += $$POINTING/pointing/debug/pointing.lib
    }else{
      LIBS += -L$$POINTING/pointing/release -lpointing
      PRE_TARGETDEPS += $$POINTING/pointing/release/pointing.lib
    }
  }else {
    CONFIG(debug, debug|release){
      LIBS += -L$$POINTING/pointing/debug -lpointing
      PRE_TARGETDEPS += $$POINTING/pointing/debug/libpointing.a
    }else{
      LIBS += -L$$POINTING/pointing/release -lpointing
      PRE_TARGETDEPS += $$POINTING/pointing/release/libpointing.a
    }
  }

  CONFIG += windows console
  LIBS   += -L$$POINTING/pointing/libs/windows -ldinput8 -ldxguid -lsetupapi -lgdi32 -lwbemuuid  -lAdvapi32 -luser32# -lcomsupp
}
