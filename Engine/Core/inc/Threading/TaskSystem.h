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
		void ParallelFor(const u32 workGroupSize, std::vector<T>& vec, std::function<void(T&)> func)
		{
			const u32 vecSize = static_cast<u32>(vec.size());
			if (vecSize == 0)
			{
				return;
			}

			// Store all the start indexes to be completed.
			std::queue<u32> startIdxs;
			std::mutex startIdxsMutex;

			const u32 taskNum = IntDivideRoundUp(vecSize, workGroupSize);
			for (size_t taskIdx = 1; taskIdx < taskNum; ++taskIdx)
			{
				startIdxs.push(workGroupSize * taskIdx);
			}
			// Don't add index 0 as the caller thread will handle this range (0->workGroupSize). This should mean that the caller thread
			// "always" has some of the most amount of work to do so we aren't wasting a lot of time just waiting.
			// The caller thread should have some of the highest amount of work to do other wise it will be waiting for worker threads. 
			// Really we want the worker threads to have less work so they can finish early and then move onto other work which has been queued.
			//startIdxs.push(0);

			std::vector<std::shared_ptr<Task>> tasks;
			tasks.reserve(taskNum);

			// Minus 1, as one of the worker threads will be the caller thread. The caller thread shouldn't be just waiting for work from other threads
			// it should also be helping.
			const u32 workerThreads = std::min(IntDivideRoundUp(vecSize, workGroupSize), TaskSystem::Instance().GetThreadCount()) - 1;
			if (workerThreads > 0)
			{
				for (size_t threadIdx = 0; threadIdx < workerThreads; ++threadIdx)
				{
					// Kick off all our tasks. These will run on objects vec[0] + workGroupSize.
					tasks.push_back(TaskSystem::Instance().CreateTask([&]()
						{
							while (true)
							{
								u32 startIdx = 0;
								{
									std::lock_guard l(startIdxsMutex);
									if (startIdxs.empty())
									{
										break;
									}
									startIdx = startIdxs.front();
									startIdxs.pop();
								}
								const u32 endIdx = std::min(startIdx + workGroupSize, vecSize);
								IS_PROFILE_SCOPE("ParallelFor");
								ZoneTextF("ParallelFor (%d)", endIdx - startIdx);

								for (size_t i = startIdx; i < endIdx; ++i)
								{
									func(vec[i]);
								}
							}
						}));
				}
			}

			// Make sure the caller thread always does the items from 0->workGroupSize so it "should" have a "full"
			// load of work.
			bool completedIndexZero = false;
			while (true)
			{
				u32 startIdx = 0;
				if (completedIndexZero)
				{
					std::lock_guard l(startIdxsMutex);
					if (startIdxs.empty())
					{
						break;
					}
					startIdx = startIdxs.front();
					startIdxs.pop();
				}

				completedIndexZero = true;
				const u32 endIdx = std::min(startIdx + workGroupSize, vecSize);
				IS_PROFILE_SCOPE("ParallelFor");
				ZoneTextF("ParallelFor (%d)", endIdx - startIdx);

				for (size_t i = startIdx; i < endIdx; ++i)
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