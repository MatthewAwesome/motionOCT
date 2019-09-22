import ctypes as c
import numpy as np
from numpy.ctypeslib import ndpointer
import matplotlib.pyplot as plt
import time

lib = c.CDLL('motionOCT')

print(lib)

d = np.load('C:/Users/sstucker/OneDrive/Documents/BOAS_OCT_motion_corr/data/fig8_13/fig8_13_raw_5000nm_2p0.npy')
p = np.load('C:/Users/sstucker/OneDrive/Documents/BOAS_OCT_motion_corr/data/fig8_13/fig8_13_pos_5000nm_2p0.npy')

print('Data loaded.')

flattened = d.flatten()

t0 = flattened[0:2048*40].astype(np.uint16)

B1 = p[2,:].astype(np.bool)
B2 = p[3,:].astype(np.bool)

print('Length of B1, B2',len(B1),len(B2))

x = 10
n = 40

window = np.ones(2048,dtype=np.float32)

bool_p = ndpointer(dtype=np.bool,ndim=1,flags='C_CONTIGUOUS')
uint16_p = ndpointer(dtype=np.uint16,ndim=1,flags='C_CONTIGUOUS')
float_p = ndpointer(dtype=np.float32,ndim=1,flags='C_CONTIGUOUS')
double_p = ndpointer(dtype=np.float64,ndim=1,flags='C_CONTIGUOUS')
fftw_complex_p = ndpointer(dtype=np.complex64,ndim=1,flags='C_CONTIGUOUS')

class FramePreprocessor(c.Structure):
    pass

FramePreprocessorHandle = c.POINTER(FramePreprocessor)

lib.initPreprocessor.argtypes = [c.c_int, c.c_int, bool_p, bool_p, float_p, double_p]
lib.initPreprocessor.restype = FramePreprocessorHandle

lib.preprocessFrame.argtypes = [FramePreprocessorHandle, uint16_p, fftw_complex_p]

lib.delPreprocessor.argtypes = [FramePreprocessorHandle]


print('Doing some processing!')

out = np.empty(2*1024*x,dtype=np.complex64)

lam = np.load('lam.npy').astype(np.float64)

processor = lib.initPreprocessor(x,n,B1,B2,window,lam)
print('Processor initiated')

number_of_frames = 1000

times = []
outs = []
for i in range(number_of_frames):
    start = time.time()
    lib.preprocessFrame(processor,t0,out)
    outs.append(out)
    times.append(time.time()-start)

print(str(np.mean(times)*1000)[0:5],'ms',str(1/(np.mean(times)))[0:5],'hz averaged over',number_of_frames,'frames')

lib.delPreprocessor(processor)
print('Processor deleted')

plt.figure(1)
plt.title('2 * 10 A-scan B-scan output (real dB)')
plt.plot(np.real(outs[0]))

image = np.empty([2048,10],dtype=np.complex64)
for i in range(10):
    image[:,i] = outs[0][2048*i:2048*i+2048]

plt.figure(2)
plt.title('B-scan (dB)')
plt.imshow(20*np.log10(np.abs(np.real(image[0:120,:]))),aspect=0.6)

plt.show()
