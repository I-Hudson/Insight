#include "Threading/Semaphore.h"

namespace Insight
{
	Semaphore::Semaphore(int val)
		: currVal(val), maxVal(16)
	{ }

	void Semaphore::Wait()
	{
		std::unique_lock<std::mutex> lk(mtx);
		cv.wait(lk, [&]() {return currVal > 0; });
		--currVal;
		return;
	}

	void Semaphore::Signal()
	{
		std::unique_lock<std::mutex> lk(mtx);
		currVal = std::min<unsigned short>(currVal + 1u, maxVal);
		cv.notify_one();
	}
}