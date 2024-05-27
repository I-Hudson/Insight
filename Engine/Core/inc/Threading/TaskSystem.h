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
#include <functional>
#include <algorithm>

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

			u32 GetThreadCount() const { return static_cast<u32>(m_threads.size()); }

			template<typename Func, typename... Args>
			static auto CreateTask(Func func, Args&&... args)
			{
				using ResultType = std::invoke_result_t<Func, Args...>;
				TaskResult<ResultType>* taskResult = New<TaskResult<ResultType>, Core::MemoryAllocCategory::Threading>();
				TaskFuncWrapper<ResultType, Func, Args...>* taskWrapper = New<TaskFuncWrapper<ResultType, Func, Args...>>(taskResult, std::move(func), std::move(args)...);

				std::shared_ptr<TaskWithResult<ResultType>> taskShared = std::make_shared<TaskWithResult<ResultType>>(taskResult, taskWrapper);
			
				if (TaskSystem::Instance().m_threads.size() > 0)
				{
					std::unique_lock lock(TaskSystem::Instance().m_mutex);
					TaskSystem::Instance().m_queuedTasks.push(taskShared);
					lock.unlock();
				}
				else
				{
					taskShared->Call();
				}
				return taskShared;
			}

		private:
			bool GetTask(std::shared_ptr<Task>& task);
			static void ThreadWorker(ThreadData threadData);

		private:
			std::mutex m_mutex;
			std::queue<std::shared_ptr<Task>> m_queuedTasks;
			std::unordered_set<Task*> m_runningTasks;
			std::vector<Thread> m_threads;

			std::atomic<bool> m_destroy = false;
		};

		template<typename T>
		void ParallelFor(u32 workGroupSize, std::vector<T>& vec, std::function<void(T&)> func)
		{
			const u32 vecSize = static_cast<u32>(vec.size());
			if (vecSize == 0)
			{
				return;
			}

			const u32 neededThreadNum = IntDivideRoundUp(vecSize, workGroupSize); // This includes the caller thread.
			const u32 availableThreadNum = TaskSystem::Instance().GetThreadCount() + 1; // This includes the caller thread.
			const u32 threadsNum = std::min(neededThreadNum, availableThreadNum);

			if (neededThreadNum > availableThreadNum)
			{
				workGroupSize = IntDivideRoundUp(vecSize, threadsNum); // Always round up so there is no left over work.
			}

			std::vector<std::shared_ptr<Task>> tasks;
			tasks.reserve(threadsNum - 1);

			for (size_t threadIdx = 1; threadIdx < threadsNum; ++threadIdx)
			{
				const u32 startIdx = workGroupSize * threadIdx;
				const u32 endIdx = std::min(startIdx + workGroupSize, vecSize);
				// Kick off all our tasks. These will run on objects vec[0] + workGroupSize.
				tasks.push_back(TaskSystem::Instance().CreateTask([startIdx, endIdx, &vec, &func]()
					{
						IS_PROFILE_SCOPE("ParallelFor");
						ZoneTextF("ParallelFor (%d)", endIdx - startIdx);
						for (size_t i = startIdx; i < endIdx; ++i)
						{
							func(vec[i]);
						}
					}));
			}

			{
				IS_PROFILE_SCOPE("ParallelFor");
				ZoneTextF("ParallelFor (%d)", workGroupSize);
				// The caller thread works on 0->workGroupSize
				for (size_t i = 0; i < workGroupSize; ++i)
				{
					func(vec[i]);
				}
			}

			// Wait for all other tasks to complete.
			for (size_t taskIdx = 0; taskIdx < tasks.size(); ++taskIdx)
			{
				IS_PROFILE_SCOPE("Wait");
				tasks[taskIdx]->Wait();
			}
		}
	}
}