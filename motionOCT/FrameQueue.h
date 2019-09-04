#ifndef FRAME_QUEUE_H
#define FRAME_QUEUE_H

#include <mutex>
#include <condition_variable>
#include <queue>

class FrameQueue
{
	public:

		uint16_t* pop()
		{
			std::unique_lock<std::mutex> mlock(mutex_);  //mutex scope lock
			while (queue_.empty())  // needed to prevent spurious wakes
			{
				cv_.wait(mlock);  // wait on condition variable cv_ while queue is empty
			}
			uint16_t* item = queue_.front();
			queue_.pop();
			return item;
		}

		void push(uint16_t * item)
		{
			std::unique_lock<std::mutex> mlock(mutex_);
			queue_.push(item);
			mlock.unlock();
			cv_.notify_one();  //tell a single waiting thread to proceed
		}

	private:
		std::queue<uint16_t*> queue_;
		std::mutex mutex_;
		std::condition_variable cv_;


};

#endif
