# file: curi.pxd

from libcpp.string cimport string
from libcpp cimport bool

cdef extern from "pointing/utils/URI.h" namespace "pointing":

    cdef cppclass URI:
        URI()
        URI(URI src)
        URI(string s)
        URI(char *s)
        bool resemble(URI &other)
        void clear()
        void load(string &uri)
        bool isEmpty()
        void generalize()
        string asString()
