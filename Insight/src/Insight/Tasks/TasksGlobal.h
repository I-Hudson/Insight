#pragma once

#include "Insight/Core.h"

#include <deque>
#include <array>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <memory>

namespace Insight
{
	namespace Tasks
	{

		/*
			https://github.com/delscorcho/basic-job-system/blob/master/jobsystem.h
		*/

		class TaskManager;		
		class TaskSystemWorker;
		class TaskState;

		/**
		 * Global system components.
		 */
		std::atomic<size_t>             s_nextJobId = 0;    ///< Job ID assignment for debugging / profiling.
		std::mutex                      s_signalLock;       ///< Global mutex for worker signaling.
		std::condition_variable         s_signalThreads;    ///< Global condition var for worker signaling.
		std::atomic<size_t>             s_activeWorkers = 0;

		uint64_t GetBit(uint64_t n) { return static_cast<uint64_t>(1) << n; }
		typedef std::function<void()> TaskDelegate;          ///< Structure of callbacks that can be requested as tasks.
		typedef uint64_t affinity_t;
		static const affinity_t kAffinityAllBits = static_cast<affinity_t>(~0);
		typedef std::shared_ptr<class TaskState> TaskStatePtr;

		enum TaskEvent
		{
			TaskEvent_TaskPopped,            ///< A task was popped from a queue.
			TaskEvent_TaskStart,             ///< A task is about to start.
			TaskEvent_TaskDone,              ///< A task just completed.
			TaskEvent_TaskRun,               ///< A task has been completed.
			TaskEvent_TaskRunAssisted,       ///< A task has been completed through outside assistance.
			TaskEvent_TaskStolen,            ///< A worker has stolen a job from another worker. 
			TaskEvent_WorkerAwoken,          ///< A worker has been awoken.
			TaskEvent_WorkerUsed,            ///< A worker has been utilized.
		};

		struct TaskQueueEntry;

		typedef std::function<void(const TaskQueueEntry& job, TaskEvent, uint64_t, size_t)> TaskEventObserver;  ///< Delegate definition for task event observation.
		typedef std::deque<TaskQueueEntry> TaskQueue;     ///< Data structure to represent task queue.

		inline affinity_t CalculateSafeWorkerAffinity(size_t workerIndex, size_t workerCount)
		{
			affinity_t affinity = kAffinityAllBits; // Set all bits so jobs with affinities out of range can still be processed.
			affinity &= ~(workerCount - 1);         // Wipe bits within valid range.
			affinity |= GetBit(workerIndex);        // Set worker-specific bit.

			return affinity;
		}
	}
}
