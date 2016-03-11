from cpointingdevice cimport PointingDevice 
from cdisplaydevice cimport DisplayDevice
from curi cimport URI
from libcpp cimport bool

cdef extern from "pointing/transferfunctions/SubPixelFunction.h" namespace "pointing::SubPixelFunction":

    ctypedef long long int	 inttime "pointing::TimeStamp::inttime" 

cdef extern from "pointing/transferfunctions/SubPixelFunction.h" namespace "pointing":

    cdef cppclass SubPixelFunction:
        SubPixelFunction(URI &uri, URI &funcURI, PointingDevice*, DisplayDevice* )

        void applyi(int input_dx, int input_dy, int* dxPixel, int* dyPixel, inttime timestamp)
        void applyd(int input_dx, int input_dy, double* dxPixel, double* dyPixel, inttime timestamp)
        void clearState()
        void setCardinalitySize(int cardinality, int size)
        void setHumanResolution(int humanResolution)
        void setSubPixeling(bool subpixeling)
