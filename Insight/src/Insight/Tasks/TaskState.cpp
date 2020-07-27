#include "ispch.h"

#include "Insight/Tasks/TaskState.h"
#include "Insight/Log.h"

namespace Insight
{
	namespace Tasks
	{
		TaskState::TaskState()
			: m_debugChar(0)
		{
			m_taskId = s_nextJobId++;
			m_workerAffinity = kAffinityAllBits;

			m_dependencies.store(0, std::memory_order_release);
			m_cancel.store(0, std::memory_order_release);
			m_ready.store(0, std::memory_order_release);
			m_done.store(0, std::memory_order_release);
		}

		TaskState::~TaskState()
		{
		}

		TaskState& TaskState::SetReady()
		{
			IS_CORE_ASSERT(IsDone(), "[TaskState::SetReady] Task is not done!");

			m_cancel.store(false, std::memory_order_relaxed);
			m_ready.store(true, std::memory_order_release);

			s_signalThreads.notify_all();

			return *this;
		}

		TaskState& TaskState::Cancel()
		{
			IS_CORE_ASSERT(IsDone(), "[TaskState::Cancel] Task is not done!");

			m_cancel.store(true, std::memory_order_relaxed);

			return *this;
		}

		TaskState& TaskState::AddDependant(TaskStatePtr dependant)
		{
			IS_CORE_ASSERT(m_dependants.end() == std::find(m_dependants.begin(), m_dependants.end(), dependant), "[TaskState::AddDependant] dependant already exists!");

			m_dependants.push_back(dependant);

			dependant->m_dependencies.fetch_add(1, std::memory_order_relaxed);

			return *this;
		}

		TaskState& TaskState::SetWorkerAffinity(affinity_t affinity)
		{
			m_workerAffinity = affinity ? affinity : kAffinityAllBits;
			return *this;
		}

		bool TaskState::IsDone() const
		{
			return m_done.load(std::memory_order_acquire);
		}

		bool TaskState::Wait(size_t maxWaitMicroSeconds)
		{
			if (!IsDone())
			{
				std::unique_lock<std::mutex> lock(m_doneMutex);

				if (maxWaitMicroSeconds == 0)
				{
					m_doneSignal.wait(lock,
						[this]()
						{
							return IsDone();
						}
					);
				}
				else
				{
					m_doneSignal.wait_for(lock, std::chrono::microseconds(maxWaitMicroSeconds));
				}
			}

			return IsDone();
		}

		bool TaskState::AreDependenciesMet() const
		{
			if (!m_ready.load(std::memory_order_acquire))
			{
				return false;
			}

			if (m_dependencies.load(std::memory_order_relaxed) > 0)
			{
				return false;
			}

			return true;
		}

		bool TaskState::HasDependencis() const
		{
			return (m_dependencies.load(std::memory_order_relaxed) > 0);
		}

		void TaskState::SetQueued()
		{
			m_done.store(false, std::memory_order_release);
		}

		void TaskState::SetDone()
		{
			IS_CORE_ASSERT(!IsDone(), "[TaskState::SetDone] Task has not completed!");

			for (auto it = m_dependants.begin(); it != m_dependants.end(); ++it)
			{
				(*it)->m_dependencies.fetch_sub(1, std::memory_order_relaxed);
			}

			std::lock_guard<std::mutex> lock(m_doneMutex);
			m_done.store(true, std::memory_order_release);
			m_doneSignal.notify_all();
		}

		bool TaskState::AwaitingCancellation() const
		{
			return m_cancel.load(std::memory_order_relaxed);
		}
	}
}

