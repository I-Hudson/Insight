#include "ispch.h"

#include "Insight/Tasks/TaskSystemWorker.h"
#include "Insight/Tasks/TaskState.h"
#include "Insight/Instrumentor/Instrumentor.h"

namespace Insight
{
	namespace Tasks
	{
		TaskSystemWorker::TaskSystemWorker(const TaskWorkerDescriptor& desc, const TaskEventObserver& eventObserver)
			: m_allWorkers(nullptr)
			, m_workerCount(0)
			, m_workerIndex(0)
			, m_desc(desc)
			, m_eventObserver(eventObserver)
			, m_hasShutDown(false)
		{
		}

		TaskSystemWorker::~TaskSystemWorker()
		{
		}

		void TaskSystemWorker::Start(size_t index, TaskSystemWorker** allWorkers, size_t workerCount)
		{
			m_workerIndex = index;
			m_allWorkers = allWorkers;
			m_workerCount = workerCount;

			m_thread = std::thread(&TaskSystemWorker::WorkerThreadProc, this);
		}

		void TaskSystemWorker::Shutdown()
		{
			m_stop.store(true, std::memory_order_relaxed);

			while (!m_hasShutDown.load(std::memory_order_acquire))
			{
				s_signalThreads.notify_all();

				std::this_thread::sleep_for(std::chrono::microseconds(100));
			}

			if (m_hasShutDown.load(std::memory_order_acquire))
			{
				m_thread.join();
			}
		}

		TaskStatePtr TaskSystemWorker::CreateTask(TaskDelegate delegate)
		{
			TaskQueueEntry entry = { delegate, std::make_shared<TaskState>() };
			entry.m_state->SetQueued();

			{
				std::lock_guard<std::mutex> queueLock(m_queueLock);
				m_queue.insert(m_queue.begin(), entry);
			}

			return entry.m_state;
		}

		void TaskSystemWorker::NotifyEventObserver(const TaskQueueEntry& task, TaskEvent event, uint64_t workerIndex, size_t taskId)
		{
#ifdef IS_PROFILE
			if (m_eventObserver)
			{
				m_eventObserver(task, event, workerIndex, taskId);
			}
#endif
		}

		bool TaskSystemWorker::RemoveTaskFromQueue(TaskQueue& queue, TaskQueueEntry& task, bool& hasUnsatisfiedDependencies, affinity_t workerAffinity)
		{
			for (auto it = queue.begin(); it != queue.end();)
			{
				const TaskQueueEntry& candidate = (*it);

				if ((workerAffinity & candidate.m_state->m_workerAffinity) != 0)
				{
					if (candidate.m_state->AwaitingCancellation())
					{
						candidate.m_state->SetDone();
						it = queue.erase(it);

						continue;
					}
					else if (candidate.m_state->AreDependenciesMet())
					{
						task = candidate;
						queue.erase(it);

						NotifyEventObserver(task, TaskEvent_TaskPopped, m_workerIndex);

						return true;
					}
				}

				hasUnsatisfiedDependencies = true;
				++it;
			}

			return false;
		}

		bool TaskSystemWorker::NextJob(TaskQueueEntry& task, bool& hasUnsatisfiedDependencies, bool useWorkStealing, affinity_t workerAffinity)
		{
			bool foundJob = false;

			{
				std::lock_guard<std::mutex> queueLock(m_queueLock);
				foundJob = RemoveTaskFromQueue(m_queue, task, hasUnsatisfiedDependencies, workerAffinity);
			}

			if (!foundJob && useWorkStealing)
			{
				for (size_t i = 0; foundJob == false && i < m_workerCount; ++i)
				{
					IS_CORE_ASSERT(m_allWorkers[i], "[TaskSystemWorker::NextJob] Worker is nullptr!");
					TaskSystemWorker& worker = *m_allWorkers[i];

					{
						std::lock_guard<std::mutex> queueLock(worker.m_queueLock);
						foundJob = RemoveTaskFromQueue(worker.m_queue, task, hasUnsatisfiedDependencies, workerAffinity);
					}
				}

				if (foundJob)
				{
					NotifyEventObserver(task, TaskEvent_TaskStolen, m_workerIndex);
				}
			}

			return foundJob;
		}

		void TaskSystemWorker::SetThreadName(const std::string& name)
		{
			(void)name;
#ifdef  IS_PLATFORM_WINDOWS
			typedef struct tagTHREADNAME_INFO
			{
				unsigned long dwType; // must be 0x1000
				const char* szName; // pointer to name (in user addr space)
				unsigned long dwThreadID; // thread ID (-1=caller thread)
				unsigned long dwFlags; // reserved for future use, must be zero
			} THREADNAME_INFO;

			THREADNAME_INFO threadName;
			threadName.dwType = 0x1000;
			threadName.szName = name.c_str();
			threadName.dwThreadID = GetCurrentThreadId();
			threadName.dwFlags = 0;
			__try
			{
				RaiseException(0x406D1388, 0, sizeof(threadName) / sizeof(ULONG_PTR), (ULONG_PTR*)&threadName);
			}
			__except (EXCEPTION_CONTINUE_EXECUTION)
			{
			}
#endif
		}

		void TaskSystemWorker::WorkerThreadProc()
		{
			SetThreadName(m_desc.m_name.c_str());

#ifdef IS_PLATFORM_WINDOWS
			SetThreadAffinityMask(m_thread.native_handle(), m_desc.m_cpuAffinity);
#endif // IS_PLATFORM_WINDOWS

			const affinity_t workerAffinity = CalculateSafeWorkerAffinity(m_workerIndex, m_workerCount);

			while (true)
			{
				TaskQueueEntry task;
				{
					std::unique_lock<std::mutex> signalLock(s_signalLock);

					bool hasUnsatisfiedDependencies;

					while (!m_stop.load(std::memory_order_relaxed) &&
						!NextJob(task, hasUnsatisfiedDependencies, m_desc.m_enableWorkStealing, workerAffinity))
					{
						s_signalThreads.wait(signalLock);
						NotifyEventObserver(task, TaskEvent_WorkerAwoken, m_workerIndex);
					}
				}

				if (m_stop.load(std::memory_order_relaxed))
				{
					m_hasShutDown.store(true, std::memory_order_release);

					break;
				}

				s_activeWorkers.fetch_add(1, std::memory_order_acq_rel);
				{
					NotifyEventObserver(task, TaskEvent_WorkerUsed, m_workerIndex);

					NotifyEventObserver(task, TaskEvent_TaskStart, m_workerIndex, task.m_state->m_taskId);
					task.m_delegate();
					NotifyEventObserver(task, TaskEvent_TaskDone, m_workerIndex);

					task.m_state->SetDone();

					NotifyEventObserver(task, TaskEvent_TaskRun, m_workerIndex);

					s_signalThreads.notify_one();
				}
				s_activeWorkers.fetch_sub(1, std::memory_order_acq_rel);
			}
		}
	}
}