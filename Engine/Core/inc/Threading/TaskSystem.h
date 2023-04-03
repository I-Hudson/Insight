#pragma once

#include "Core/Singleton.h"
#include "Core/ISysytem.h"
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
		class IS_CORE TaskSystem : public Core::Singleton<TaskSystem>, public Core::ISystem
		{
		public:
			IS_SYSTEM(TaskSystem);

			virtual void Initialise() override;
			virtual void Shutdown() override;

			template<typename Func, typename... Args>
			static auto CreateTask(Func func, Args&&... args)
			{
				using ResultType = std::invoke_result_t<Func, Args...>;
				TaskResult<ResultType>* taskResult = New<TaskResult<ResultType>, Core::MemoryAllocCategory::Threading>();
				TaskFuncWrapper<ResultType, Func, Args...>* taskWrapper = new TaskFuncWrapper<ResultType, Func, Args...>(taskResult, std::move(func), std::move(args)...);
				TrackPtr(taskWrapper);

				TaskWithResultShared<ResultType> taskShared = MakeRPtr<TaskWithResult<ResultType>>(taskResult, taskWrapper);
				std::unique_lock lock(TaskSystem::Instance().m_mutex);
				TaskSystem::Instance().m_queuedTasks.push(taskShared);
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