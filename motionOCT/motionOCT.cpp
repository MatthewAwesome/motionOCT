// motionOCT.cpp : Defines the exported functions for the DLL application.
#include "stdafx.h"
//#include "FrameQueue.h"
//#include "ThreadPool.h"
#include "FrameProcessor.h"
//#include <fftw3.h>
//#include <thread>
//#include <iostream>

// C++ functions must be wrapped with C interfaces to be accessible via ctypes.
extern "C" 
{
	
	__declspec(dllexport) FramePreprocessor*  initPreprocessor(int a_per_b, int a_per_scan, bool * b_index_1, bool * b_index_2, float * apodization_window, double * wavelength_array)
	{
		return new FramePreprocessor(a_per_b, a_per_scan, b_index_1, b_index_2, apodization_window, wavelength_array);
	}

	__declspec(dllexport) void preprocessFrame(FramePreprocessor* preprocessor, uint16_t * in, fftwf_complex * out)
	{
		preprocessor->preprocessFrame(in, out);
	}

	__declspec(dllexport) void delPreprocessor(FramePreprocessor* preprocessor)
	{
		delete preprocessor;
	}

}

