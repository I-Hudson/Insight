#pragma once

#include "Insight/Core.h"
#include "Insight/Tasks/TasksGlobal.h"

#include <deque>
#include <array>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

namespace Insight
{
	namespace Tasks
	{
		struct TaskQueueEntry
		{
			TaskDelegate m_delegate;
			TaskStatePtr m_state;
		};

		class TaskSystemWorker;
		class TaskManager;

		class TaskState
		{
		public:
			TaskState();
			~TaskState();

			TaskState& SetReady();
			TaskState& Cancel();
			TaskState& AddDependant(TaskStatePtr dependant);
			TaskState& SetWorkerAffinity(affinity_t affinity);

			bool IsDone() const;
			bool Wait(size_t maxWaitMicroSeconds = 0);
			bool AreDependenciesMet() const;
			bool HasDependencis() const;

		private:
			void SetQueued();
			void SetDone();
			bool AwaitingCancellation() const;

			std::atomic<bool> m_cancel;
			std::atomic<bool> m_ready;
			std::atomic<bool> m_done;

			std::condition_variable m_doneSignal;
			std::mutex m_doneMutex;

			std::vector<TaskStatePtr> m_dependants;
			std::atomic<int> m_dependencies;

			affinity_t m_workerAffinity;
			uint64_t m_taskId;
			char m_debugChar;

			friend class TaskSystemWorker;
			friend class TaskManager;
		};
	}
}