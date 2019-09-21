#ifndef FRAME_QUEUE_H
#define FRAME_QUEUE_H

#include <mutex>
#include <condition_variable>
#include <queue>

template <typename T, int size>
class FrameQueue
{
	public:

		void pop(T * out)
		{
			std::unique_lock<std::mutex> mlock(mutex_);  //mutex scope lock
			while (queue_.empty())  // needed to prevent spurious wakes
			{
				cv_.wait(mlock);  // wait on condition variable cv_ while queue is empty
			}
			for (int i = 0; i < size; i++) {
				out[i] = queue_.front();
				queue_.pop();
			}
		}

		void push(T in)
		{
			std::unique_lock<std::mutex> mlock(mutex_);
			for (int i = 0; i < size; i++) {
				queue_.push(in[i]);
			}
			mlock.unlock();
			cv_.notify_one();  //tell a single waiting thread to proceed
		}

	private:
		std::queue<T> queue_;
		std::mutex mutex_;
		std::condition_variable cv_;

};

#endif
