#include "Threading/TaskManager.h"

namespace Insight
{
	namespace Threading
	{

		void TaskManager::Init(u32 threadCount)
		{
			Destroy();

			m_destroy = false;
			if (threadCount == 0)
			{
				threadCount = std::thread::hardware_concurrency();
			}

			std::lock_guard lock(m_mutex);
			m_threads.resize(threadCount);
			for (size_t i = 0; i < m_threads.size(); ++i)
			{
				m_threads.at(i).SetThreadIndex(static_cast<u8>(i));
				m_threads.at(i).Spwan(ThreadWorker, ThreadData(this, &m_threads.at(i)));
			}
		}

		void TaskManager::Destroy()
		{
			m_destroy = true;
			std::lock_guard lock(m_mutex);
			for (size_t i = 0; i < m_threads.size(); ++i)
			{
				m_threads.at(i).Join();
			}
			m_threads.clear();
		}

		bool TaskManager::GetTask(TaskSharedPtr& task)
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

		void TaskManager::ThreadWorker(ThreadData threadData)
		{
			while (!threadData.TaskManager->m_destroy.load())
			{
				TaskSharedPtr task;
				if (threadData.TaskManager->GetTask(task))
				{
					task->Call();
					{
						std::lock_guard lock(threadData.TaskManager->m_mutex);
						threadData.TaskManager->m_runningTasks.erase(task.Get());
					}
					task.Reset();
				}
				threadData.Thread->SleepFor(16);
			}
		}
	}
}