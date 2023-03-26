#include "Threading/TaskSystem.h"

namespace Insight
{
	namespace Threading
	{

		void TaskSystem::Initialise()
		{
			Shutdown();
			m_destroy = false;

			u32 threadCount = std::thread::hardware_concurrency();
			std::lock_guard lock(m_mutex);
			m_threads.resize(threadCount);
			for (size_t i = 0; i < m_threads.size(); ++i)
			{
				m_threads.at(i).SetThreadIndex(static_cast<u8>(i));
				m_threads.at(i).Spwan(ThreadWorker, ThreadData(this, &m_threads.at(i)));
			}
			m_state = Core::SystemStates::Initialised;
		}

		void TaskSystem::Shutdown()
		{
			m_destroy = true;
			for (size_t i = 0; i < m_threads.size(); ++i)
			{
				m_threads.at(i).Join();
			}

			std::lock_guard lock(m_mutex);
			m_queuedTasks = {};
			m_threads.clear();
			m_state = Core::SystemStates::Not_Initialised;
		}

		bool TaskSystem::GetTask(TaskSharedPtr& task)
		{
			std::lock_guard lock(m_mutex);
			if (m_queuedTasks.size() > 0)
			{
				task = m_queuedTasks.front();
				m_queuedTasks.pop();
				m_runningTasks.insert(task.Get());
				return true;
			}
			else
			{
				return false;
			}
		}

		void TaskSystem::ThreadWorker(ThreadData threadData)
		{
			while (!threadData.TaskSystem->m_destroy.load())
			{
				TaskSharedPtr task;
				if (threadData.TaskSystem->GetTask(task))
				{
					task->Call();
					{
						std::lock_guard lock(threadData.TaskSystem->m_mutex);
						threadData.TaskSystem->m_runningTasks.erase(task.Get());
					}
					task.Reset();
				}
				threadData.Thread->SleepFor(16);
			}
		}
	}
}