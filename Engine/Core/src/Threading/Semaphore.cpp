#include "Threading/Semaphore.h"

namespace Insight
{
	Semaphore::Semaphore(int val)
		: currVal(val), maxVal(65535)
	{ }

	void Semaphore::Wait()
	{
		std::unique_lock<std::mutex> lk(mtx);
		cv.wait(lk, [&]() {return currVal > 0; });
		--currVal;
		return;
	}

	void Semaphore::Signal(const u32 count)
	{
		std::unique_lock<std::mutex> lk(mtx);
		currVal = std::min<unsigned short>(currVal + count, maxVal);
		for (size_t i = 0; i < count; i++)
		{
			cv.notify_one();
		}
	}
}