#pragma once
#include "Insight/Core.h"
#include "Insight/Tasks/TasksGlobal.h"
#include "Insight/Tasks/TaskState.h"

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
		struct TaskWorkerDescriptor
		{
            TaskWorkerDescriptor(const char* name = "TaskWorkerDescriptor", unsigned int cpuAffinity = 0xffffffff, bool enableWorkSteeling = true)
                : m_name(name)
                , m_cpuAffinity(cpuAffinity)
                , m_enableWorkStealing(enableWorkSteeling)
            {
            }

            std::string     m_name;                     ///< Worker name, for debug/profiling displays.
            unsigned int    m_cpuAffinity;              ///< Thread affinity. Defaults to all cores.
            bool            m_enableWorkStealing : 1;   ///< Enable queue-sharing between workers?
		};

        class TaskManager;

        class TaskSystemWorker
        {
        public:
            TaskSystemWorker(const TaskWorkerDescriptor& desc, const TaskEventObserver& eventObserver);
            ~TaskSystemWorker();

            void Start(size_t index, TaskSystemWorker** allWorkers, size_t workerCount);
            void Shutdown();

            TaskStatePtr CreateTask(TaskDelegate delegate);

        private:

            void NotifyEventObserver(const TaskQueueEntry& task, TaskEvent event, uint64_t workerIndex, size_t taskId = 0);
            bool RemoveTaskFromQueue(TaskQueue& queue, TaskQueueEntry& task, bool& hasUnsatisfiedDependencies, affinity_t workerAffinity);
            bool NextJob(TaskQueueEntry& task, bool& hasUnsatisfiedDependencies, bool useWorkStealing, affinity_t workerAffinity);

            void SetThreadName(const std::string& name);
            void WorkerThreadProc();

            std::thread                 m_thread;                   ///< Thread instance for worker.
            std::atomic<bool>           m_stop;                     ///< Has a stop been requested?
            std::atomic<bool>           m_hasShutDown;              ///< Has the worker completed shutting down?

            mutable std::mutex          m_queueLock;                ///< Mutex to guard worker queue.
            TaskQueue                   m_queue;                    ///< Queue containing requested jobs.

            TaskSystemWorker**          m_allWorkers;               ///< Pointer to array of all workers, for queue-sharing / work-stealing.
            size_t                      m_workerCount;              ///< Number of total workers (size of m_allWorkers array).
            size_t                      m_workerIndex;              ///< This worker's index within m_allWorkers.

            TaskEventObserver            m_eventObserver;            ///< Observer of job-related events occurring on this worker.
            TaskWorkerDescriptor         m_desc;                     ///< Descriptor/configuration of this worker.

            friend TaskManager;
        };
	}
}