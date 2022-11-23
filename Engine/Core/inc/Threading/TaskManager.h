#pragma once

#include "Core/Singleton.h"
#include "Core/TypeAlias.h"

#include "Threading/Task.h"
#include "Threading/Thread.h"

#include <mutex>
#include <queue>
#include <unordered_set>
#include <memory>

namespace Insight
{
	namespace Threading
	{
		class IS_CORE TaskManager : public Core::Singleton<TaskManager>
		{
		public:

			void Init(u32 threadCount = 0);
			void Destroy();

			template<typename Func, typename... Args>
			static auto CreateTask(Func func, Args&&... args)
			{
				TaskFuncWrapper<Func, Args...>* taskWrapper = new TaskFuncWrapper<Func, Args...>(std::move(func), std::move(args)...);//NewArgsTracked(TaskFuncWrapper<ResultType, Func, Args...>, std::move(func), std::move(args)...);
				TrackPtr(taskWrapper);
				TaskSharedPtr taskShared = NewArgsTracked(Task, taskWrapper);
				std::unique_lock lock(TaskManager::Instance().m_mutex);
				TaskManager::Instance().m_queuedTasks.push(taskShared);
				lock.unlock();
				return taskShared;
			}

		private:
			bool GetTask(TaskSharedPtr& task);
			static void ThreadWorker(ThreadData threadData);

		private:
			std::mutex m_mutex;
			std::queue<TaskSharedPtr> m_queuedTasks;
			std::unordered_set<Task*> m_runningTasks;
			std::vector<Thread> m_threads;

			std::atomic<bool> m_destroy = false;
		};
	}
}