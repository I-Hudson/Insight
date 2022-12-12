#include "Threading/Thread.h"

#include "Core/Profiler.h"

#ifdef _WIN32
#include <Windows.h>
#endif

namespace Insight
{
	namespace Threading
	{
		ThreadData::ThreadData(Insight::Threading::TaskSystem* taskManager, Insight::Threading::Thread* thread)
			: TaskSystem(taskManager)
			, Thread(thread)
		{ }

		Thread::Thread(Thread&& other)
		{
			m_handle = std::move(other.m_handle);
			m_id = std::move(other.m_id);
			m_tls = std::move(other.m_tls);
			m_threadData = std::move(other.m_threadData);
			m_callback = std::move(other.m_callback);
		}

		Thread::~Thread()
		{
			Join();
		}

		void Thread::Spwan(Callback callback, ThreadData threadData)
		{
			if (HasSpawned())
			{
				Join();
			}

			m_threadData = threadData;
			m_callback = callback;
			m_handle = std::thread([this]()
				{
					if (m_callback != nullptr)
					{
						IS_PROFILE_THREAD("WorkerThread");
						m_callback(m_threadData);
					}
				});
			m_id = m_handle.get_id();

		}

		void Thread::SetAffinity(size_t i)
		{
			if (HasSpawned())
			{
#ifdef _WIN32
				DWORD_PTR mask = 1ull << i;
				SetThreadAffinityMask(m_handle.native_handle(), mask);
#endif
				m_tls.SetAffinity = true;
			}
		}

		void Thread::SleepFor(u32 milliSeconds)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(milliSeconds));
		}

		void Thread::Join()
		{
			if (HasSpawned())
			{
				m_handle.join();
				m_handle = std::thread();
				m_id = std::thread::id();

				m_tls = { };
			}
		}
		
		void Thread::SetThreadIndex(u8 threadIndex)
		{
			m_tls.ThreadIndex = threadIndex;
		}
	}
}