# tests/tests.pro --
#
# Initial software
# Authors: Izzatbek Mukhanov
# Copyright © Inria

TEMPLATE = subdirs

SUBDIRS += interpolation subpixel hidreportparser

# FIXME tests are not passing on linux
# Need to change linuxHIDPointingDevice
macx:windows {
  SUBDIRS += pointingdevice
}
