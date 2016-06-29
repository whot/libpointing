from cpointingdevice cimport PointingDevice 
from cdisplaydevice cimport DisplayDevice

cdef extern from "pointing/transferfunctions/TransferFunction.h" namespace "pointing::TransferFunction":

    ctypedef long long int	 inttime "pointing::TimeStamp::inttime" 
        
    cdef TransferFunction* create(char*, PointingDevice*, DisplayDevice* ) 

cdef extern from "pointing/transferfunctions/TransferFunction.h" namespace "pointing":

    cdef cppclass TransferFunction:
        void applyi(int input_dx, int input_dy, int* dxPixel, int* dyPixel, inttime timestamp)
        void applyd(int input_dx, int input_dy, double* dxPixel, double* dyPixel, inttime timestamp)
        void clearState()

    