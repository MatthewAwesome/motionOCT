import ctypes as c
import numpy as np
import matplotlib.pyplot as plt
import time
import motionOCT

lib = c.CDLL('motionOCT')

print(lib)

# Load some test data!

d = np.load('C:/Users/sstucker/OneDrive/Documents/BOAS_OCT_motion_corr/data/fig8_13/fig8_13_raw_5000nm_2p0.npy')
p = np.load('C:/Users/sstucker/OneDrive/Documents/BOAS_OCT_motion_corr/data/fig8_13/fig8_13_pos_5000nm_2p0.npy')
flattened = d.flatten()
t0 = flattened[0:2048*40].astype(np.uint16)  # The first B-scan for use with the demo

n = 40  # Total A-scans in input (not all are included with B)
x = 10  # A-scans in output

print('Data loaded.')

# Create b matrix, the matrix of A-scans to process and keep

B1 = p[2,:].astype(np.bool)
B2 = p[3,:].astype(np.bool)
b = np.logical_or(B1,B2)

# Load chirp matrix, containing wavelengths corresponding to spectrum bins for lambda->k interpolation
lam = np.load('lam.npy').astype(np.float64)

# Define apodization window
window = np.hanning(2048).astype(np.float32)

# Initialize preprocessor
number_of_threads = 4
preprocessor = motionOCT.init_preprocessor(n,b,window,lam,number_of_threads)
print('Processor initiated with',number_of_threads,'threads!')


# Repeat many calculations and benchmark performance
number_of_frames = 1000

times = []
outs = []

out = np.empty(2*1024*x,dtype=np.complex64)

for i in range(number_of_frames):
    start = time.time()
    motionOCT.preprocess_frame(preprocessor,t0,out)
    outs.append(out)
    times.append(time.time()-start)

print(str(np.mean(times)*1000)[0:5],'ms',str(1/(np.mean(times)))[0:5],'hz averaged over',number_of_frames,'frames')

motionOCT.delete_preprocessor(preprocessor)
print('Processor deleted.')

plt.figure(1)
plt.title('2 * 10 A-scan B-scan output (|Re|)')
plt.plot(np.abs(np.real(outs[0])))
plt.ylim(0,15000)

# Visualize some output

image = np.empty([1024,10],dtype=np.complex64)
for i in range(10):
    image[:,i] = outs[0][1024*i:1024*i+1024]

plt.figure(2)
plt.title('B-scan (dB)')
plt.imshow(20*np.log10(np.abs(np.real(image[0:120,:]))),aspect=0.6)

plt.show()
