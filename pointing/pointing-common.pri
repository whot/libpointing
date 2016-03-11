# pointing/pointing-common.pri --
#
# Initial software
# Authors: Nicolas Roussel
# Copyright © INRIA

#message("Using pointing-common.pri")

macx {
  # CONFIG   += x86 x86_64
  # QMAKE_MAC_SDK = macosx10.9
  # QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.10
}

# remove possible other optimization flags
QMAKE_CXXFLAGS_RELEASE -= -O
QMAKE_CXXFLAGS_RELEASE -= -O1
QMAKE_CXXFLAGS_RELEASE -= -O2

# add the desired -O3 if not present
!win32-msvc* {
    QMAKE_CXXFLAGS_RELEASE *= -O3
}

windows:!win32-msvc* {
    QMAKE_LFLAGS += -static -lpthread -static-libgcc -static-libstdc++
}
