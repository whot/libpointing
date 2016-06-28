# -*- coding: utf-8 -*-

import threading
from libcpp cimport bool
from libcpp.string cimport string
cimport ctransferfunction 
cimport csubpixelfunction 
cimport cdisplaydevice
cimport cpointingdevice
cimport cpointingdevicemanager
cimport curi

PointingDeviceManager_Init = False
  
cdef extern from "Python.h":
     ctypedef int PyGILState_STATE
     PyGILState_STATE PyGILState_Ensure()
     void PyGILState_Release(PyGILState_STATE gstate)
     void PyEval_InitThreads()
     
	 
cdef void defaultCallback(void *context, cpointingdevice.inttime timestamp, 
                                      int dx, int dy, 
                                      int buttons):
        cdef PyGILState_STATE st
        st = PyGILState_Ensure()
        
        self=<PointingDevice>context
        if self.pycallback_:
            self.pycallback_(timestamp, dx, dy, buttons)
       
        PyGILState_Release(st)


cdef void deviceUpdateCallback(void *context,
                                const cpointingdevicemanager.PointingDeviceDescriptor &desc,
                                bool wasAdded):
        cdef PyGILState_STATE st
        st = PyGILState_Ensure()
        
        pyDesc = PointingDeviceDescriptor(desc.devURI.asString(), desc.vendor, desc.product, desc.vendorID, desc.productID)
        if wasAdded:
            PointingDeviceManager.descriptors[pyDesc.devURI] = pyDesc
        else:
            PointingDeviceManager.descriptors.pop(pyDesc.devURI, None)

        for cb in PointingDeviceManager.callbacks:
            cb(pyDesc, wasAdded)
       
        PyGILState_Release(st)


cdef class URI(object):
    cdef curi.URI *thiscptr_ 
    def __cinit__(self, uri=""):
        b = uri.encode("utf-8")
        cdef char *s = b
        self.thiscptr_ = new curi.URI(s)

    def __dealloc__(self):
        del self.thiscptr_

    cdef bool resemble(self, curi.URI &other):
        """Compares only scheme, opaque, host, port and path"""
        return self.thiscptr_.resemble(other)

    def clear(self):
        self.thiscptr_.clear()

    cdef load(self, string &uri):
        self.thiscptr_.load(uri)
        
    def isEmpty(self):
        return self.thiscptr_.isEmpty()

    def generalize(self):
        self.thiscptr_.generalize()

    def asString(self):
        return str(self)

    def __str__(self):
        return bytes(<char *>self.thiscptr_.asString().c_str()).decode()
    

cdef class PointingDevice(object):
    cdef cpointingdevice.PointingDevice *thiscptr_
    cdef object pycallback_
    
    def __cinit__(self, uri):
        PyEval_InitThreads()
        pycallback_=None
        self.thiscptr_ = cpointingdevice.create(uri)
        self.thiscptr_.setPointingCallback(defaultCallback, <void *>self)

    def __dealloc__(self):
        del self.thiscptr_
    
    def getResolution(self, defval=None):
        return self.thiscptr_.getResolution(NULL)
    resolution=property(getResolution, None, None)
    
    def getUpdateFrequency(self):
        return self.thiscptr_.getUpdateFrequency(NULL)
    updatefrequency=property(getUpdateFrequency, None, None)

    def getVendor(self):
        return self.thiscptr_.getVendor()
    vendor=property(getVendor, None, None)

    def getVendorID(self):
        return self.thiscptr_.getVendorID()
    vendorID=property(getVendorID, None, None)

    def getProduct(self):
        return self.thiscptr_.getProduct()
    product=property(getProduct, None, None)

    def getProductID(self):
        return self.thiscptr_.getProductID()
    productID=property(getProductID, None, None)

    def setCallback(self, cb):
        self.pycallback_=cb
   
    cdef cpointingdevice.PointingDevice* getRawPointer(self):
        return self.thiscptr_
    
    def getURI(self, expanded=False):
        cdef curi.URI uri = self.thiscptr_.getURI(expanded)
        # FIXME: I couldn't pass to the (python) URI constructor the
        # (c++) URI object (like a copy constructor) so I had to pass
        # the string representation and then construct a new object
        # from it.
        cdef string s = uri.asString()
        pyuri = URI()
        pyuri.load(s)
        return pyuri
    uri=property(getURI, None, None)
    
    @classmethod
    def create(self, uri):
        try:
            return PointingDevice(uri)
        except:
            # For Python 3.2
            return PointingDevice(bytes(uri,"utf-8"))

    @classmethod
    def idle(self, milliseconds):
        cpointingdevice.idle(milliseconds)    

# Pseudo-singleton class
cdef class PointingDeviceManager(object):
    callbacks = set()
    descriptors = {}

    def __cinit__(self):
        #PyEval_InitThreads()
        global PointingDeviceManager_Init
        if not PointingDeviceManager_Init:
            cpointingdevicemanager.get().addDeviceUpdateCallback(deviceUpdateCallback, <void *>NULL)
            PointingDeviceManager_Init = True

    def addDeviceUpdateCallback(self, cb):
        self.callbacks.add(cb)

    def removeDeviceUpdateCallback(self, cb):
        self.callbacks.remove(cb)
    
    @classmethod
    def get(self):
        return PointingDeviceManager()

    def __iter__(self):
        for uri, desc in self.descriptors.items():
            yield desc

class PointingDeviceDescriptor(object):
    def __init__(self, devURI, vendor, product, vendorID, productID):
        self.devURI = devURI
        self.vendor = vendor
        self.product = product
        self.vendorID = vendorID
        self.productID = productID

    def __str__(self):
        return "Device\n\turi=%s\n\tvendor=%s\n\tproduct=%s\n\tvendorID=%d\n\tproductID=%d"%(self.devURI, self.vendor, self.product, self.vendorID, self.productID)

    def __eq__(self, other):
        return isinstance(other, PointingDeviceDescriptor) and self.devURI==other.devURI and self.vendor==other.vendor and self.product == other.product and self.vendorID==other.vendorID and self.productID==other.productID

    def __ne__(self, other):
        return not self==other

      
class Size(object):
    def __init__(self, width, height):
        self.width = width
        self.height = height

    def __str__(self):
        return "%gx%g"%(self.width, self.height)

    def __eq__(self, other):
        return isinstance(other, Size) and self.width==other.width and self.height==other.height

    def __ne__(self, other):
        return not self==other

class Point(object):
    def __init__(self, x, y):
        self.x = x
        self.y = y

    def __str__(self):
        return "(%g,%g)"%(self.x, self.y)

    def __eq__(self, other):
        return isinstance(other, Point) and self.x==other.x and self.y==other.y

    def __ne__(self, other):
        return not self==other

class Bounds(object):
    def __init__(self, x, y, width, height):
        self.origin = Point(x, y)
        self.size = Size(width, height)

    def __str__(self):
        return "origin=%s size=%s"%(self.origin, self.size)

    def __eq__(self, other):
        return isinstance(other, Bounds) and self.origin==other.origin and self.size==other.size

    def __ne__(self, other):
        return not self==other

cdef class DisplayDevice(object):    
    cdef cdisplaydevice.DisplayDevice *thiscptr_
    def __cinit__(self, uri):
        b = uri.encode("utf-8")
        self.thiscptr_ = cdisplaydevice.create(b)

    def __dealloc__(self):
        del self.thiscptr_
    
    def getBounds(self):
        cdef cdisplaydevice.Bounds b
        b=self.thiscptr_.getBounds(NULL)
        return Bounds(b.origin.x, b.origin.y, b.size.width, b.size.height)
    bounds=property(getBounds, None, None)
        
    def getSize(self):
        cdef cdisplaydevice.Size s
        s=self.thiscptr_.getSize(NULL)
        return Size(s.width, s.height)        
    size=property(getSize, None, None)    

    def getResolution(self):
        cdef double hdpi = 0
        cdef double vdpi = 0
        cdef double res = 0
        res = self.thiscptr_.getResolution(&hdpi, &vdpi, NULL)
        if hdpi==0 and vdpi==0:
            return res, res
        else:
            return hdpi, vdpi
    resolution=property(getResolution, None, None)

    def getRefreshRate(self):
        return self.thiscptr_.getRefreshRate(NULL)
    refreshrate=property(getRefreshRate, None, None)

    def getURI(self, expanded=False):
        cdef curi.URI uri = self.thiscptr_.getURI(expanded)
        # FIXME: I couldn't pass to the (python) URI constructor the
        # (c++) URI object (like a copy constructor) so I had to pass
        # the string representation and then construct a new object
        # from it.
        cdef string s = uri.asString()
        pyuri = URI()
        pyuri.load(s)
        return pyuri
    uri=property(getURI, None, None)
       
    cdef cdisplaydevice.DisplayDevice* getRawPointer(self):
        return self.thiscptr_
        
    @classmethod
    def create(self, uri):
        if str(uri).startswith("dummy:"):
            return DummyDisplayDevice(uri)
        else:
            return DisplayDevice(uri)

cdef class DummyDisplayDevice(DisplayDevice):    
     
    def setBounds(self, bounds):
        cdef cdisplaydevice.Bounds b
        b.origin.x = float(bounds.origin.x)
        b.origin.y = float(bounds.origin.y)
        b.size.width = float(bounds.size.width)
        b.size.height = float(bounds.size.height)
        cdef cdisplaydevice.DummyDisplayDevice *d
        d = <cdisplaydevice.DummyDisplayDevice *>self.thiscptr_
        d.setBounds(b)

    def setSize(self, size):
        cdef cdisplaydevice.Size s
        s.width = float(size.width)
        s.height = float(size.height)
        cdef cdisplaydevice.DummyDisplayDevice *d
        d = <cdisplaydevice.DummyDisplayDevice *>self.thiscptr_
        d.setSize(s)        

    def setRefreshRate(self, refreshrate):
        cdef cdisplaydevice.DummyDisplayDevice *d
        d = <cdisplaydevice.DummyDisplayDevice *>self.thiscptr_
        d.setRefreshRate(int(refreshrate))

    def setResolution(self, resolution):
        cdef cdisplaydevice.DummyDisplayDevice *d
        d = <cdisplaydevice.DummyDisplayDevice *>self.thiscptr_
        d.setResolution(float(resolution))
        
   
cdef class TransferFunction(object):
    cdef ctransferfunction.TransferFunction *thiscptr_

    def __cinit__(self, uri, pdev, ddev):     
        #self.pdev_=pdev
        #self.ddev_=ddev
        cdef PointingDevice tpdev
        cdef DisplayDevice tddev
        
        tpdev=pdev
        tddev=ddev
		
        cdef cpointingdevice.PointingDevice* iptr
        cdef cdisplaydevice.DisplayDevice* dptr
        
        iptr=tpdev.thiscptr_
        dptr=tddev.thiscptr_
        self.thiscptr_ = ctransferfunction.create(uri, iptr, 
                                                       dptr)

    def __dealloc__(self):
        del self.thiscptr_
     
    def applyi(self, dx, dy, timestamp):
        cdef int dxPixel=0
        cdef int dyPixel=0
        self.thiscptr_.applyi(dx, dy, &dxPixel, &dyPixel, timestamp)
        return (dxPixel, dyPixel)

    def applyd(self, dx, dy, timestamp):
        cdef double dxPixel=0
        cdef double dyPixel=0
        self.thiscptr_.applyd(dx, dy, &dxPixel, &dyPixel, timestamp)
        return (dxPixel, dyPixel)
    
    def clearState(self):
        self.thiscptr_.clearState()
    
    @classmethod
    def create(self, uri, pointingdevice, displaydevice):
        try:
            return TransferFunction(uri, pointingdevice, displaydevice)
        except:
            # For Python 3.2
            return TransferFunction(bytes(uri,"utf-8"), pointingdevice, displaydevice)

"""
cdef class SubPixelFunction(object):
    cdef csubpixelfunction.SubPixelFunction *thiscptr_

    def __cinit__(self, uri, func_uri, pdev, ddev):
        cdef PointingDevice tpdev
        cdef DisplayDevice tddev
        cdef URI tfunc_uri, t_uri
        
        tpdev=pdev
        tddev=ddev
        p_func_uri = URI(func_uri)
        p_uri = URI(uri)
        
        cdef cpointingdevice.PointingDevice* iptr
        cdef cdisplaydevice.DisplayDevice* dptr
        cdef curi.URI *uptr
        cdef curi.URI *ufptr
        
        iptr=tpdev.thiscptr_
        dptr=tddev.thiscptr_
        uptr=p_uri.thiscptr_
        ufptr=p_func_uri.thiscptr_
        self.thiscptr_ = new csubpixelfunction.SubPixelFunction(uptr[0], ufptr[0], iptr, dptr)

    def __dealloc__(self):
        del self.thiscptr_

    def applyi(self, dx, dy, timestamp):
        cdef int dxPixel=0
        cdef int dyPixel=0
        self.thiscptr_.applyi(dx, dy, &dxPixel, &dyPixel, timestamp)
        return (dxPixel, dyPixel)

    def applyd(self, dx, dy, timestamp):
        cdef double dxPixel=0
        cdef double dyPixel=0
        self.thiscptr_.applyd(dx, dy, &dxPixel, &dyPixel, timestamp)
        return (dxPixel, dyPixel)
    
    def clearState(self):
        self.thiscptr_.clearState()

    def setCardinalitySize(self, cardinality, size):
        self.thiscptr_.setCardinalitySize(cardinality, size)

    def setHumanResolution(self, res_human):
        self.thiscptr_.setHumanResolution(res_human)

    def setSubPixeling(self, subpixeling):
        self.thiscptr_.setSubPixeling(subpixeling)
"""
