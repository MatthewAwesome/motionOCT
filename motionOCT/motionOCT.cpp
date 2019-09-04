// motionOCT.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "FrameQueue.h"
#include "ThreadPool.h"
#include <fftw3.h>
#include <thread>

class FramePreprocessor
{
	public:

	FramePreprocessor()
	{
		ThreadPool threadPool_ = ThreadPool(4);
		// Dummy arrays for declaration of plan-- plan is called via new-array execution functions
		fftwf_complex in[2048], out[2048];
		fftwf_plan fft_plan_ = fftwf_plan_dft_1d(2048, in, out, FFTW_BACKWARD, FFTW_PATIENT);
	}

	// TODO implement other constructors

	void enqueueFrame(uint16_t * frame)
	{
		frameQueue_.push(frame);
	}

	private:
		FrameQueue frameQueue_;

};

void preprocessFrame(uint16_t * frame, int N)
{

}

extern "C" __declspec(dllexport) void rfft(fftwf_complex * in, fftwf_complex * out) {

	fftwf_plan p = fftwf_plan_dft_1d(2048, in, out, FFTW_BACKWARD, FFTW_ESTIMATE);

	fftwf_execute(p);

	fftwf_destroy_plan(p);

	// Normalization
	for (int i = 0; i < 2048; i++) {
		out[i][0] *= 1./2048;
		out[i][1] *= 1./2048;
	}

}