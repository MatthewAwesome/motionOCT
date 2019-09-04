import ctypes as c
import numpy as np
from numpy.ctypeslib import ndpointer
import matplotlib.pyplot as plt

lib = c.CDLL('motionOCT')

print(lib)

lib.sum.argtypes = [c.c_int, c.c_int]
lib.sum.restype = c.c_int

print('2 + 2 =',lib.sum(2,2))

in_arr = np.zeros(2048,dtype=np.complex64)
in_arr[400:2048-400] = 2

out_arr = np.empty(2048,dtype=np.complex64)

out_npy = np.fft.ifft(in_arr)

plt.figure(1)
plt.plot(np.real(in_arr))

fftw_arr = ndpointer(dtype=np.complex64,ndim=1,flags='C_CONTIGUOUS')
lib.rfft.argtypes = [fftw_arr,fftw_arr]

lib.rfft(in_arr,out_arr)

plt.figure(2)
plt.plot(out_arr)
plt.plot(out_npy)

plt.show()