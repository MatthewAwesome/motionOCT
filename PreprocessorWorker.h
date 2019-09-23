#pragma once

#include <iostream>
#include "stdafx.h"
#include <fftw3.h>

class PreprocessorWorker
{
	public:
			
		PreprocessorWorker(int input_size, float * apodization_window, double * wavelength_array)
		{	

			ready_ = 0; // Not ready until FFT plan and interpolation matrix is created successfully

			n = input_size;
			lam = wavelength_array;
			apod = apodization_window;

			for (int i = 0; i < 2048; i++)  // Normalize apodization window
			{
				
				apod[i] = apod[i];  // TODO: figure out if normalization is necessary

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

			// FFT input array initialization
			b_apodized = fftwf_alloc_real(2048 * n);
			dummy_out = fftwf_alloc_complex(1024 * n);

			// Point in and out pointers for FFT at appropriate arrays
			in_p = b_apodized;
			out_p = dummy_out;

			// FFT arrays for declaration of plan-- plan is called via new-array execution functions
			fft_plan_ = fftwf_plan_dft_r2c_1d(2048, in_p, out_p, FFTW_PATIENT);

			//Free the dummy array used to create FFT plan, real input array is passed in
			fftwf_free(dummy_out);

			if (fft_plan_ == NULL)
			{
				std::cout << "WARNING: Plan not created" << std::endl;
			}
			else {

				ready_ = 1;

			}

		}

		~PreprocessorWorker()
		{

			fftwf_destroy_plan(fft_plan_);
			fftwf_free(b_apodized);

		}

		void preprocess(uint16_t* in, fftwf_complex* out)
		{
			// Takes pointers to input_size and input_size/2 arrays in and out

			for (int i = 0; i < n; i++)  // Until entire raw frame is looked at
			{

				for (int j = 0; j < 2048; j++)
				{

					b_apodized[2048 * i + j] = lambda_k_interp(&in[2048 * i],j) * apod[j];

				}

				// Point input and output to each A-scan
				in_p = &b_apodized[2048*i];
				out_p = &out[1024*i];

				fftwf_execute_dft_r2c(fft_plan_, in_p, out_p);  // Compute FFT

			}

		}

		bool is_ready()
		{
			return ready_;
		}

	private:

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

		bool ready_;

		fftwf_plan fft_plan_;
		int n;
		float * apod;
		double * lam;

		double d_lam;
		double k[2048];
		int nn[2048][2];
		float * b_apodized;

		float* in_p;
		fftwf_complex* out_p;
		fftwf_complex* dummy_out;


};

