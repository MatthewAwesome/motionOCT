// motionOCT.cpp : Defines the exported functions for the DLL application.
#include "stdafx.h"
//#include "FrameQueue.h"
//#include "ThreadPool.h"
#include "FramePreprocessor.h"
//#include <fftw3.h>
//#include <thread>
//#include <iostream>

// C++ functions must be wrapped with C interfaces to be accessible via ctypes.
extern "C" 
{
	
	__declspec(dllexport) PreprocessorWorker*  init_worker(int input_size, float * apodization_window, double * wavelength_array)
	{
		return new PreprocessorWorker(input_size, apodization_window, wavelength_array);
	}

	__declspec(dllexport) void worker_preprocess(PreprocessorWorker* preprocessor, uint16_t * in, fftwf_complex * out)
	{
		preprocessor->preprocess(in, out);
	}

	__declspec(dllexport) void delete_worker(PreprocessorWorker* worker)
	{
		delete worker;
	}

	__declspec(dllexport) FramePreprocessor* init_preprocessor(int a_per_frame, bool * b_array, float * apodization_window, double * wavelength_array, int number_of_workers)
	{
		return new FramePreprocessor(a_per_frame, b_array, apodization_window, wavelength_array, number_of_workers);
	}

	__declspec(dllexport) void preprocess_frame(FramePreprocessor* preprocessor, uint16_t * in, fftwf_complex * out)
	{
		preprocessor->process_frame(in, out);
	}

	__declspec(dllexport) void delete_preprocessor(FramePreprocessor* preprocessor)
	{
		delete preprocessor;
	}

}

