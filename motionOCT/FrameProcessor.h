#pragma once

#include <iostream>
#include "stdafx.h"
//#include "ThreadPool.h"
#include <fftw3.h>
#include <complex>

class FramePreprocessor
{
	public:
		
		int x;
		int n;
		bool * b1;
		bool * b2;
		float * apod;

		float* in_p;
		fftwf_complex* out_p;
			
		FramePreprocessor(int a_per_b, int a_per_scan, bool * b_index_1, bool * b_index_2, float * apodization_window)
		{	

			x = a_per_b;
			n = a_per_scan;
			b1 = b_index_1;
			b2 = b_index_2;
			apod = apodization_window;

			// FFT arrays for declaration of plan-- plan is called via new-array execution functions

			fft_plan_ = fftwf_plan_dft_r2c_1d(2048, in_p, out_p, FFTW_ESTIMATE);
			if (fft_plan_ == NULL)
			{
				std::cout << "WARNING: Plan not created" << std::endl;
			}

		}

		~FramePreprocessor()
		{

			fftwf_destroy_plan(fft_plan_);

		}

		void preprocessFrame(uint16_t* frame, fftwf_complex* out)
		{

			float * b1_b2_apodized;
			b1_b2_apodized = fftwf_alloc_real(2 * 2048 * x);

			int ib1 = 0;
			int ib2 = x;

			for (int a = 0; a < n; a++)  // Until entire raw frame is looked at, or both B-scans are extracted
			{

				if (b1[a] == 1)
				{

					for (int i = 0; i < 2048; i++)
					{

						b1_b2_apodized[2048 * ib1 + i] = frame[2048 * a + i] * apod[i] * 1 / 2048;

					}

					in_p = &b1_b2_apodized[2048*ib1];
					out_p = &out[1024*ib1];

					fftwf_execute_dft_r2c(fft_plan_, in_p, out_p);  // Compute FFT

					ib1 += 1;

				}

				else if (b2[a] == 1)
				{

					for (int i = 0; i < 2048; i++)
					{

						b1_b2_apodized[2048 * ib2 + i] = frame[2048 * a + i] * apod[i] * 1 / 2048;

					}
					
					in_p = &b1_b2_apodized[2048*ib2];
					out_p = &out[1024*ib2];

					fftwf_execute_dft_r2c(fft_plan_, in_p, out_p);  // Compute FFT

					ib2 += 1;

				}

			}

			fftwf_free(b1_b2_apodized);

		}

	private:

		fftwf_plan fft_plan_;


};

//in_p[k] = frame[j+k] * apod[k] * 1 / 2048;  // apodization + normalization, loading of FFT matrix for given A-scan
							//fftwf_execute(fft_plan_);  // Compute FFT
