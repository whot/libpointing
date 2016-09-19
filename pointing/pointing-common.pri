# pointing/pointing-common.pri --
#
# Initial software
# Authors: Nicolas Roussel
# Copyright © INRIA

#message("Using pointing-common.pri")

# Remove these optimization flags...
QMAKE_CXXFLAGS_RELEASE -= -O
QMAKE_CXXFLAGS_RELEASE -= -O1
QMAKE_CXXFLAGS_RELEASE -= -O2 
# ... and add -O3 if not present (might not work on all platforms)
win32-msvc* {
  QMAKE_CXXFLAGS_RELEASE *= -O2
} else {
  QMAKE_CXXFLAGS_RELEASE *= -O3
}

macx {
    # Needed for IOHIDDeviceRegisterInputReportWithTimeStampCallback
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.10
}

# Compile with c++11 on linux (Qt5/Qt4)
unix:!macx {
    QMAKE_CXXFLAGS += -std=c++11
}

# Compile with mingw
win32-g++ {
    QMAKE_CXXFLAGS += -std=gnu++0x
    QMAKE_LFLAGS += -static -lpthread -static-libgcc -static-libstdc++
}
