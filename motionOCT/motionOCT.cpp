// motionOCT.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <fftw3.h>
#include <iostream>

extern "C" __declspec(dllexport) int sum(int a, int b) {
	return a + b;
}

extern "C" __declspec(dllexport) void rfft(fftwf_complex * in, fftwf_complex * out) {

	for (int i = 0; i < 2048; i++) {
		std::cout << in[i][0] << ", " << in[i][1] << std::endl;
	}

	fftwf_plan p = fftwf_plan_dft_1d(2048, in, out, FFTW_BACKWARD, FFTW_ESTIMATE);

	fftwf_execute(p);

	fftwf_destroy_plan(p);

	for (int i = 0; i < 2048; i++) {
		out[i][0] *= 1./2048;
		out[i][1] *= 1./2048;
		std::cout << out[i][0] << ", " << out[i][1] << std::endl;
	}

}