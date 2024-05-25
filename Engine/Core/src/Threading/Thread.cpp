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
						IS_PROFILE_THREAD("InsightWorkerThread");
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

		void Thread::SetName(const char* threadName)
		{
#ifdef _WIN32
			const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push,8)
			typedef struct tagTHREADNAME_INFO
			{
				DWORD dwType; // Must be 0x1000.
				LPCSTR szName; // Pointer to name (in user addr space).
				DWORD dwThreadID; // Thread ID (-1=caller thread).
				DWORD dwFlags; // Reserved for future use, must be zero.
			} THREADNAME_INFO;
#pragma pack(pop)

			THREADNAME_INFO info;
			info.dwType = 0x1000;
			info.szName = threadName;
			info.dwThreadID = *reinterpret_cast<DWORD*>(&m_handle.get_id());
			info.dwFlags = 0;

			__try
			{
				RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
			}
#endif
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