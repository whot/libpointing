# file: pointingdevice.pxd

from libcpp cimport bool   
from libcpp.string cimport string
from curi cimport URI   

cdef extern from "pointing/input/PointingDeviceManager.h" namespace "pointing::PointingDeviceManager":

    cdef PointingDeviceManager* get()
   
cdef extern from "pointing/input/PointingDeviceManager.h" namespace "pointing":

    struct PointingDeviceDescriptor:
            string devURI
            string name
            int vendorID
            int productID

    ctypedef void (*DeviceUpdateCallback)(void *context,
                                          const PointingDeviceDescriptor &desc,
                                          bool wasAdded)

    cdef cppclass PointingDeviceManager:
        void addDeviceUpdateCallback(DeviceUpdateCallback callback, void *context)
        void removeDeviceUpdateCallback(DeviceUpdateCallback callback, void *context)