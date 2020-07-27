#include "ispch.h"

#include "Insight/Tasks/TaskManager.h"
#include "Insight/Log.h"

namespace Insight
{
	namespace Tasks
	{
		TaskManager::TaskManager()
			: m_tasksRun(0)
			, m_tasksStolen(0)
			, m_usedMask(0)
			, m_awokenMask(0)
			, m_nextRoundRobinWorkerIndex(0)
		{
		}

		TaskManager::~TaskManager()
		{
			JoinWorkersAndShutdown();
		}

		bool TaskManager::Create(const TaskManagerDescriptor& desc)
		{
			JoinWorkersAndShutdown();

			m_desc = desc;

			const size_t workerCount = desc.m_workers.size();
			m_workers.reserve(workerCount);

			const TaskEventObserver observer = std::bind(
				&TaskManager::Observer, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

			// Create workers. We don't spawn the threads yet.
			for (size_t i = 0; i < workerCount; ++i)
			{
				const TaskWorkerDescriptor& workerDesc = desc.m_workers[i];

				TaskSystemWorker* worker = new TaskSystemWorker(workerDesc, observer);
				m_workers.push_back(worker);
			}

			// Start the workers (includes spawning threads). Each worker maintains
			// understanding of what other workers exist, for work-stealing purposes.
			for (size_t i = 0; i < workerCount; ++i)
			{
				m_workers[i]->Start(i, &m_workers[0], workerCount);
			}

			return !m_workers.empty();
		}

		TaskStatePtr TaskManager::AddTask(TaskDelegate delegate, char debugChar)
		{
			TaskStatePtr state = nullptr;

			// \todo - workers should maintain a tls pointer to themselves, so we can push
			// directly into our own queue.

			if (!m_workers.empty())
			{
				// Add round-robin style. Note that work-stealing helps load-balance,
				// if it hasn't been disabled. If it has we may need to consider a
				// smarter scheme here.
				state = m_workers[m_nextRoundRobinWorkerIndex]->CreateTask(delegate);
				state->m_debugChar = debugChar;

				m_nextRoundRobinWorkerIndex = (m_nextRoundRobinWorkerIndex + 1) % m_workers.size();
			}

			return state;
		}

		void TaskManager::AssistUnitTaskIsDone(TaskStatePtr state)
		{
			IS_CORE_ASSERT(state->m_ready.load(std::memory_order_acquire), "[TaskManager::AssistUnitTaskIsDone]");

			const affinity_t workerAffinity = kAffinityAllBits;

			// Steal jobs from workers until the specified job is done.
			while (!state->IsDone())
			{
				IS_CORE_ASSERT(!m_workers.empty(), "[TaskManager::AssistUnitTaskIsDone]");

				TaskQueueEntry job;
				bool hasUnsatisfiedDependencies;

				if (m_workers[0]->NextJob(job, hasUnsatisfiedDependencies, true, workerAffinity))
				{
					Observer(job, TaskEvent_TaskStart, m_workers.size(), job.m_state->m_taskId);
					job.m_delegate();
					Observer(job, TaskEvent_TaskDone, m_workers.size());

					job.m_state->SetDone();

					Observer(job, TaskEvent_TaskRunAssisted, 0);

					s_signalThreads.notify_one();
				}
			}
		}

		void TaskManager::AssistUnitTaskIsDone()
		{
			IS_CORE_ASSERT(!m_workers.empty(), "[TaskManager::AssistUnitTaskIsDone]");

			// Steal and run jobs from workers until all queues are exhausted.

			const affinity_t workerAffinity = kAffinityAllBits;

			TaskQueueEntry job;
			bool foundBusyWorker = true;

			while (foundBusyWorker)
			{
				foundBusyWorker = false;

				for (TaskSystemWorker* worker : m_workers)
				{
					if (worker->NextJob(job, foundBusyWorker, false, workerAffinity))
					{
						Observer(job, TaskEvent_TaskStart, m_workers.size(), job.m_state->m_taskId);
						job.m_delegate();
						Observer(job, TaskEvent_TaskDone, m_workers.size());

						job.m_state->SetDone();

						Observer(job, TaskEvent_TaskRunAssisted, 0);

						foundBusyWorker = true;
						s_signalThreads.notify_one();
						break;
					}
				}
			}

			for (TaskSystemWorker* worker : m_workers)
			{
				if (!worker->m_queue.empty())
				{
					IS_CORE_ASSERT(0, "[TaskManager::AssistUnitTaskIsDone]");
				}
			}
		}

		void TaskManager::JoinWorkersAndShutdown(bool finishTaks)
		{
			if (finishTaks)
			{
				AssistUnitTaskIsDone();
			}

			// Tear down each worker. Un-popped jobs may still reside in the queues at this point
			// if finishJobs = false.
			// Don't destruct workers yet, in case someone's in the process of work-stealing.
			for (size_t i = 0, n = m_workers.size(); i < n; ++i)
			{
				IS_CORE_ASSERT(m_workers[i], "[TaskManager::JoinWorkersAndShutdown]");
				m_workers[i]->Shutdown();
			}

			// Destruct all workers.
			std::for_each(m_workers.begin(), m_workers.end(), [](TaskSystemWorker* worker) { delete worker; });
			m_workers.clear();
		}

		void TaskManager::Observer(const TaskQueueEntry& task, TaskEvent event, uint64_t workerIndex, size_t taskId)
		{ }
	}
}