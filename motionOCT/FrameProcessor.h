#pragma once

#include <iostream>
#include "stdafx.h"
#include <fftw3.h>
#include <complex>

class FramePreprocessor
{
	public:
			
		FramePreprocessor(int a_per_b, int a_per_scan, bool * b_index_1, bool * b_index_2, float * apodization_window, double * wavelength_array)
		{	

			x = a_per_b;
			n = a_per_scan;
			b1 = b_index_1;
			b2 = b_index_2;
			lam = wavelength_array;
			apod = apodization_window;

			for (int i = 0; i < 2048; i++)  // Normalize apodization window
			{
				apod[i] = apod[i] * 1 / 2048;
			}

			// Populate linear-in-wavenumber array
			double lam_max = lam[2048-1];
			double lam_min = lam[0];
			double d_lam = lam_max - lam_min;
			double d_k = (1 / lam_min - 1 / lam_max) / 2048;

			for (int i = 2047; i > -1; i-=1)
			{
				k[i] = 1 / ((1 / lam_max) + d_k * i); 
			}

			//(Naively, but only once) find nearest upper and lower indices for linear interpolation
			for (int i = 0; i < 2048; i++)
			{

				int nearestj;  // Holds index of nearest-yet wavelength
				double nearest = 2048;  // Far larger than any possible distance

				for (int j = 1; j < 2047; j++)  // Can exclude first and last lambda-- all k are within these
				{

					double distance = lam[j] - k[i];

					if (std::abs(distance) < std::abs(nearest))  // If new nearest neighbor found
					{

						nearest = distance;
						nearestj = j;

					}

				}

				if (nearest >= 0)  // Distance greater than 0 means closest point is to the right
				{
					nn[i][0] = nearestj - 1;  // nn holds upper and lower interpolation bounds for all k
					nn[i][1] = nearestj;
				}

				else
				{
					nn[i][0] = nearestj;
					nn[i][1] = nearestj + 1;
				}

			}

			// FFT arrays for declaration of plan-- plan is called via new-array execution functions
			fft_plan_ = fftwf_plan_dft_r2c_1d(2048, in_p, out_p, FFTW_ESTIMATE);
			if (fft_plan_ == NULL)
			{
				std::cout << "WARNING: Plan not created" << std::endl;
			}

			// FFT input array initialization
			b1_b2_apodized = fftwf_alloc_real(2 * 2048 * x);

		}

		~FramePreprocessor()
		{

			fftwf_destroy_plan(fft_plan_);
			fftwf_free(b1_b2_apodized);

		}

		float lambda_k_interp(uint16_t * lambda_spectrum, int index)
		{
			// Casts to float for FFTWF. Interpolation is done w/ doubles. TODO: see if it makes a difference

			double y1 = lambda_spectrum[nn[index][0]];  // y-values from neighbors in spectrum
			double y2 = lambda_spectrum[nn[index][1]];
			double x1 = lam[nn[index][0]];  // corresponding initial wavelength
			double x = k[index];  // linear-in-wavenumber interpolation point

			if (y1 == y2)
			{
				return y1;
			}
			else
			{
				return y1 + (x - x1) / (y2 - y1) * d_lam;
			}

		}

		void preprocessFrame(uint16_t* frame, fftwf_complex* out)
		{

			int ib1 = 0;
			int ib2 = x;

			for (int a = 0; a < n; a++)  // Until entire raw frame is looked at, or both B-scans are extracted
			{

				if (b1[a] == 1)
				{

					for (int i = 0; i < 2048; i++)
					{

						b1_b2_apodized[2048 * ib1 + i] = lambda_k_interp(&frame[2048 * a],i) * apod[i];

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

						b1_b2_apodized[2048 * ib2 + i] = lambda_k_interp(&frame[2048 * a], i) * apod[i];

					}
					
					in_p = &b1_b2_apodized[2048*ib2];
					out_p = &out[1024*ib2];

					fftwf_execute_dft_r2c(fft_plan_, in_p, out_p);  // Compute FFT

					ib2 += 1;

				}

			}

		}

	private:

		fftwf_plan fft_plan_;
		int x;
		int n;
		bool * b1;
		bool * b2;
		float * apod;
		double * lam;

		double d_lam;
		double k[2048];
		int nn[2048][2];
		float * b1_b2_apodized;

		float* in_p;
		fftwf_complex* out_p;


};

