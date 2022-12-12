#pragma once

#include "Core/TypeAlias.h"

#include <thread>

namespace Insight
{
	namespace Threading
	{
		class TaskSystem;
		class Thread;

		class TLS
		{
		public:

			TLS() = default;
			~TLS() = default;

			u8 ThreadIndex = std::numeric_limits<u8>::max();
			bool SetAffinity = false;
		};

		// Data to be passed to the thread callback;
		class ThreadData
		{
		public:
			ThreadData() = default;
			ThreadData(Insight::Threading::TaskSystem* taskManager, Insight::Threading::Thread* thread);

			Insight::Threading::TaskSystem* TaskSystem = nullptr;
			Insight::Threading::Thread* Thread = nullptr;
		};

		class Thread
		{
		public:
			using Callback = void(*)(ThreadData data);

			Thread() = default;
			Thread(const Thread&) = delete;
			Thread(Thread&& other);
			virtual ~Thread();

			void Spwan(Callback callback, ThreadData threadData);
			void SetAffinity(size_t i);
			void SleepFor(u32 milliSeconds);
			void Join();

			bool HasSpawned() const { return m_id != std::thread::id(); }

		private:
			void SetThreadIndex(u8 threadIndex);

		private:
			std::thread m_handle;
			std::thread::id m_id;
			TLS m_tls;
			ThreadData m_threadData;

			Callback m_callback = nullptr;

			friend class TaskSystem;
		};
	}
}