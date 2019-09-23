# Python wrapper for dynamic library motionOCT
# There are functions in the library not wrapped here!

import ctypes as c
import numpy as np
from numpy.ctypeslib import ndpointer

path_to_dll = 'motionOCT'
lib = c.CDLL(path_to_dll)

bool_p = ndpointer(dtype=np.bool,ndim=1,flags='C_CONTIGUOUS')
uint16_p = ndpointer(dtype=np.uint16,ndim=1,flags='C_CONTIGUOUS')
float_p = ndpointer(dtype=np.float32,ndim=1,flags='C_CONTIGUOUS')
double_p = ndpointer(dtype=np.float64,ndim=1,flags='C_CONTIGUOUS')
fftw_complex_p = ndpointer(dtype=np.complex64,ndim=1,flags='C_CONTIGUOUS')

class FramePreprocessor(c.Structure):
    pass

FramePreprocessorHandle = c.POINTER(FramePreprocessor)

lib.init_preprocessor.argtypes = [c.c_int, bool_p, float_p, double_p, c.c_int]
lib.init_preprocessor.restype = FramePreprocessorHandle
def init_preprocessor(a_per_frame, b_array, apodization_window, wavelength_array, number_of_workers):
    return lib.init_preprocessor(a_per_frame, b_array, apodization_window, wavelength_array, number_of_workers)

lib.preprocess_frame.argtypes = [FramePreprocessorHandle, uint16_p, fftw_complex_p]
def preprocess_frame(preprocessor, frame, processed_frame):
    lib.preprocess_frame(preprocessor, frame, processed_frame)

lib.delete_preprocessor.argtypes = [FramePreprocessorHandle]
def delete_preprocessor(preprocessor):
    lib.delete_preprocessor(preprocessor)



