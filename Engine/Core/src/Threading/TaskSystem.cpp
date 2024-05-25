#include "Threading/TaskSystem.h"
#include "Core/Profiler.h"

namespace Insight
{
	namespace Threading
	{

		void TaskSystem::Initialise()
		{
			IS_PROFILE_FUNCTION();
			Shutdown();
			m_destroy = false;

			u32 threadCount = std::thread::hardware_concurrency();
			std::lock_guard lock(m_mutex);
			m_threads.resize(threadCount - 1);

			for (size_t threadIdx = 0; threadIdx < m_threads.size(); ++threadIdx)
			{
				Thread& thread = m_threads[threadIdx];
				thread.SetThreadIndex(static_cast<u8>(threadIdx));
				thread.Spwan(ThreadWorker, ThreadData(this, &thread));
				thread.SetName("InsightWorkerThread");
			}
			m_state = Core::SystemStates::Initialised;
		}

		void TaskSystem::Shutdown()
		{
			IS_PROFILE_FUNCTION();

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

		bool TaskSystem::GetTask(std::shared_ptr<Task>& task)
		{
			std::lock_guard lock(m_mutex);
			if (m_queuedTasks.size() > 0)
			{
				task = m_queuedTasks.front();
				m_queuedTasks.pop();
				m_runningTasks.insert(task.get());
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
				std::shared_ptr<Task> task;
				if (threadData.TaskSystem->GetTask(task))
				{
					task->Call();
					{
						std::lock_guard lock(threadData.TaskSystem->m_mutex);
						threadData.TaskSystem->m_runningTasks.erase(task.get());
					}
					task.reset();
				}
				//threadData.Thread->SleepFor(16);
			}
		}
	}
}