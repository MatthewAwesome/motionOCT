#pragma once

#include "stdafx.h"
#include "PreprocessorWorker.h"
#include <fftw3.h>
#include <thread>
#include <iostream>
#include <vector>

void preprocess_exec(PreprocessorWorker * worker, uint16_t * in, fftwf_complex * out)
{
	worker->preprocess(in, out);
}

class FramePreprocessor
{

	public:

		PreprocessorWorker ** worker_pool;
		std::thread * thread_pool;
		int n_workers;
		int n;
		bool * b;
		float * apod;
		double * lam;

		int * subarray_starts;
		int * outarray_starts;

		FramePreprocessor(int a_per_frame, bool * b_array, float * apodization_window, double * wavelength_array, int number_of_workers)
		{
			n = a_per_frame;
			b = b_array;
			apod = apodization_window;
			lam = wavelength_array;
			n_workers = number_of_workers;
			
			worker_pool = new PreprocessorWorker*[number_of_workers];
			thread_pool = new std::thread[number_of_workers];
			subarray_starts = new int[number_of_workers];
			outarray_starts = new int[number_of_workers];

			initialize_pool(number_of_workers);
		}

		~FramePreprocessor()
		{
			for (int i = 0; i < n_workers; i++)
			{
				delete worker_pool[i];
			}

			delete[] worker_pool;
			delete[] thread_pool;
			delete[] subarray_starts;
			delete[] outarray_starts;
		}
		
		void process_frame(uint16_t * in, fftwf_complex * out)
		{

			for (int i = 0; i < n_workers; i++)
			{

				thread_pool[i] = std::thread(preprocess_exec, worker_pool[i], &in[subarray_starts[i]*2048], &out[outarray_starts[i]*1024]);

			}

			for (int i = 0; i < n_workers; i++)
			{

				thread_pool[i].join();

			}

		}

	private:

		void initialize_pool(int number_of_workers)
		{

			// Populates pool of workers which work on processing subarrays of frames in parallel

			std::vector<bool> b_tmp(n);  // Version of b that will be used to assign a-lines to the workers

			// Count how many a-lines need to be processed per frame and populate b_tmp
			int alines = 0;
			for (int i = 0; i < n; i++)
			{

				b_tmp[i] = b[i];

				if (b[i])
				{
					alines++;
				}
			}

			int a_per_thread = alines / number_of_workers;
			int remainder = alines - (a_per_thread*number_of_workers);

			std::vector<int> sizes(n);

			int out_start = 0;

			for (int i = 0; i < number_of_workers; i++)
			{

				int start = -1;
				int size = a_per_thread;

				if (i == number_of_workers - 1)  // Last thread will get the remainder if number of a-lines was not evenly divisible into number of workers
				{
					size += remainder;
				}

				outarray_starts[i] = out_start; // Output array size is number of true in B
				out_start += size;  // Update out start index for next job

				int allocated = 0;
				for (int j = 0; j < n; j++)
				{

					if ((b_tmp[j]) and (allocated < size))
					{

						if (start == -1)  // Define start of subarray if not already done
						{
							start = j;
						}

						b_tmp[j] = 0;  // Set to zero so other workers don't claim the a-line
						allocated++;

					}
					
				}

				subarray_starts[i] = start;
				sizes[i] = size;

			}

			for (int i = 0; i < number_of_workers; i++)
			{

				PreprocessorWorker * worker_p = new PreprocessorWorker(sizes[i],apod,lam);  // Creates workers ready for processing of subarrays in parallel
				worker_pool[i] = worker_p;

			}

		}

};
