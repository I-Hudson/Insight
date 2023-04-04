#pragma once

#include "Threading/TaskFuncWrapper.h"
#include "Core/Memory.h"

#include <atomic>
#include <memory>

namespace Insight
{
	namespace Threading
	{
		class TaskSystem;
		class Task;
		template<typename ResultType>
		class TaskWithResult;

		using TaskSharedPtr = RPtr<Task>;
		template<typename ResultType>
		using TaskWithResultShared = RPtr<TaskWithResult<ResultType>>;

		enum class TaskStates
		{
			Queued,
			Running,
			Waiting,
			Finished,
			Canceled,
		};

		// Task with no result.
		class IS_CORE Task
		{
		public:
			Task();
			Task(ITaskFuncWrapper* funcWrapper);
			virtual ~Task();

			bool IsQueued() const { return m_state.load() == TaskStates::Queued; }
			bool IsStarted() const { return m_state.load() == TaskStates::Queued; }
			bool IsRunning() const { return m_state.load() == TaskStates::Running; }
			bool IsWaiting() const { return m_state.load() == TaskStates::Running; }
			bool IsFinished() const { return m_state.load() == TaskStates::Finished; }
			bool IsCancled() const { return m_state.load() == TaskStates::Canceled; }
			TaskStates GetState() { return m_state.load(); }

			void Wait();

		private:
			virtual void Call();

		private:
			std::atomic<TaskStates> m_state;
			ITaskFuncWrapper* m_functionWrapper = nullptr;

			std::condition_variable m_cv;
			std::mutex m_mutex;

			friend class TaskSystem;
		};

		template<typename ResultType>
		class TaskWithResult : public Task
		{
		public:
			TaskWithResult() = default;
			TaskWithResult(TaskResult<ResultType>* taskResult, ITaskFuncWrapper* funcWrapper)
				: Task(funcWrapper)
				, m_taskResult(taskResult)
			{ }
			~TaskWithResult()
			{
				Delete(m_taskResult);
			}

			TaskResult<ResultType>& GetResult() const { return *m_taskResult; }

		private:
			TaskResult<ResultType>* m_taskResult;
		};
	}
}