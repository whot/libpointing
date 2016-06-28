# -*- coding: utf-8 -*-

from pylibpointing import PointingDevice, DisplayDevice, TransferFunction
from pylibpointing import PointingDeviceManager, PointingDeviceDescriptor
#from pylibpointing import SubPixelFunction

import sys

def cb_man(desc, wasAdded):
	print desc
	print "was added" if wasAdded else "was removed"


pm = PointingDeviceManager()
PointingDevice.idle(100)
pm.addDeviceUpdateCallback(cb_man)

for desc in pm:
	print desc

"""
for desc in pm:
	print desc.devURI
	print desc.vendor, desc.product
	pdev = PointingDevice(desc.uri)
"""

pdev = PointingDevice("any:")
ddev = DisplayDevice.create("any:")
#tfct = SubPixelFunction("subpixel:?cardinality=200000&widgetsize=283&debugLevel=2", "system:", pdev, ddev)
tfct = TransferFunction("system:", pdev, ddev)

def cb_fct(timestamp, dx, dy, button):
    rx,ry=tfct.applyd(dx, dy, timestamp)
    print("%s: %d %d %d -> %d %d"%(str(timestamp), dx, dy, button, rx, ry ))
    sys.stdout.flush()

pdev.setCallback(cb_fct)
print("Move the mouse of Press CTRL+C to exit")
for i in range(0, 10000):
    PointingDevice.idle(1)
