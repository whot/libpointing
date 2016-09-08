# pointing/pointing-common.pri --
#
# Initial software
# Authors: Nicolas Roussel
# Copyright © INRIA

#message("Using pointing-common.pri")

macx {
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.10
}

unix {
    QMAKE_CXXFLAGS += -std=c++11
}

win32-g++ {
    QMAKE_CXXFLAGS += -std=gnu++0x
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
