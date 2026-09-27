#pragma once

#include "Core/Singleton.h"
#include "Core/ISysytem.h"
#include "Core/TypeAlias.h"
#include "Core/Profiler.h"

#include "Threading/Task.h"
#include "Threading/Thread.h"
#include "Threading/SpinLock.h"
#include "Threading/ScopedLock.h"

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
				IS_PROFILE_FUNCTION();

				using ResultType = std::invoke_result_t<Func, Args...>;
				TaskResult<ResultType>* taskResult = New<TaskResult<ResultType>, Core::MemoryAllocCategory::Threading>();
				TaskFuncWrapper<ResultType, Func, Args...>* taskWrapper = New<TaskFuncWrapper<ResultType, Func, Args...>>(taskResult, std::move(func), std::move(args)...);

				std::shared_ptr<TaskWithResult<ResultType>> taskShared = std::make_shared<TaskWithResult<ResultType>>(taskResult, taskWrapper);
			
				if (TaskSystem::Instance().m_threads.size() > 0)
				{
					std::unique_lock lock(TaskSystem::Instance().m_mutex);
					TaskSystem::Instance().m_queuedTasks.push(taskShared);
					lock.unlock();

					TaskSystem::Instance().m_queueCV.notify_one();
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
			IS_PROFILE_LOCKABLE_NAME(std::mutex, m_mutex, "TaskSystemQueue");
			std::condition_variable_any m_queueCV;
			std::queue<std::shared_ptr<Task>> m_queuedTasks;
			std::unordered_set<Task*> m_runningTasks;
			std::vector<Thread> m_threads;

			std::atomic<bool> m_destroy = false;
		};

		template<typename T, typename TContainer>
		void ParallelFor(std::string_view name, const u32 workGroupSize, TContainer& container, std::function<void(T&)> func)
		{
			const u32 vecSize = static_cast<u32>(container.size());
			if (vecSize == 0)
			{
				return;
			}


			std::atomic<u32> nextAvailableIdx = 0;
			const u32 taskNum = IntDivideRoundUp(vecSize, workGroupSize);

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
					tasks.push_back(TaskSystem::Instance().CreateTask([&func, &container, &nextAvailableIdx, name, vecSize, workGroupSize]()
						{
							IS_PROFILE_SCOPE("ParallelFor");
							while (true)
							{
								const u32 startIdx = nextAvailableIdx.fetch_add(workGroupSize, std::memory_order_relaxed);
								if (startIdx > vecSize)
								{
									break;
								}

								const u32 endIdx = std::min(startIdx + workGroupSize, vecSize);
								IS_PROFILE_SCOPE_TEXT("ParallelFor - %s (%d)", name.data(), endIdx - startIdx);

								for (size_t i = startIdx; i < endIdx; ++i)
								{
									func(container[i]);
								}
							}
						}));
				}
			}

			while (true)
			{
				IS_PROFILE_SCOPE("ParallelFor");

				const u32 startIdx = nextAvailableIdx.fetch_add(workGroupSize, std::memory_order_relaxed);
				if (startIdx > vecSize)
				{
					break;
				}

				const u32 endIdx = std::min(startIdx + workGroupSize, vecSize);
				IS_PROFILE_SCOPE_TEXT("ParallelFor - %s (%d)", name.data(), endIdx - startIdx);

				for (size_t i = startIdx; i < endIdx; ++i)
				{
					func(container[i]);
				}
			}

			// Wait for all other tasks to complete.
			for (size_t taskIdx = 0; taskIdx < tasks.size(); ++taskIdx)
			{
				IS_PROFILE_SCOPE("Wait");
				tasks[taskIdx]->Wait();
			}
		}

		template<typename T, typename TContainer>
		void ParallelFor(const u32 workGroupSize, TContainer& container, std::function<void(T&)> func)
		{
			ParallelFor("", workGroupSize, container, std::move(func));
		}
	}
}