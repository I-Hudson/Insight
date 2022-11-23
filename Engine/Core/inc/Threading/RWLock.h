#pragma once

#include <mutex>

namespace Insight
{
	namespace Threading
	{
		// Wrap 'T' in a lock which will automaticly set a mutex for when reading and writing.
		template<typename T>
		class RWLock
		{
			void(*WriteFunction)(T& object);

		public:
			RWLock() { }
			RWLock(const RWLock& other) = delete;
			RWLock(RWLock&& other)
				: m_object(std::move(other.m_object))
				, m_mutex(std::move(other.m_mutex))
			{ }
			~RWLock() { }

			void Set(T& object)
			{
				m_obejct = object;
			}
			T& GetRaw() const
			{
				return m_object;
			}

			void Write(WriteFunction writeFunction)
			{
				std::lock_guard lock(m_mutex);
				writeFunction(m_object);
			}

		private:
			T m_object;
			std::mutex m_mutex;
		};
	}
}