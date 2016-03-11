# file: cdisplaydevice.pxd

from libcpp cimport bool
from curi cimport URI
   
cdef extern from "pointing/output/DisplayDevice.h" namespace "pointing::DisplayDevice":

    struct Point:
            float x
            float y 
        
    struct Size:
            float width
            float height
            
    struct Bounds:
            Point origin
            Size size

    cdef DisplayDevice* create(char* my)

cdef extern from "pointing/output/DisplayDevice.h" namespace "pointing":

    cdef cppclass DisplayDevice:  
        DisplayDevice *create(char *device_uri)
        Bounds getBounds(Bounds *defval)      # pixels
        Size getSize(Size *defval)            # mm
        double getResolution(double *hdpi, double *vdpi, double *defval) # ppi
        double getRefreshRate(double *defval) # Hz
        URI getURI(bool expanded)

cdef extern from "pointing/output/DummyDisplayDevice.h" namespace "pointing":

    cdef cppclass DummyDisplayDevice:  
        void setBounds(Bounds b)
        void setSize(Size s)
        void setRefreshRate(int r)
        void setResolution(double r)
