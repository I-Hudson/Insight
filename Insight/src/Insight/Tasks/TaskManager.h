#pragma once

#include "Insight/Core.h"
#include "Insight/Tasks/TasksGlobal.h"
#include "Insight/Tasks/TaskSystemWorker.h"

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
		struct TaskManagerDescriptor
		{
			std::vector<TaskWorkerDescriptor> m_workers;             ///< Configurations for all workers that should be spawned by JobManager.
		};

		class TaskManager
		{
		public:
			TaskManager();
			~TaskManager();

			bool Create(const TaskManagerDescriptor& desc);

			TaskStatePtr AddTask(TaskDelegate delegate, char debugChar = 0);
			void AssistUnitTaskIsDone(TaskStatePtr state);
			void AssistUnitTaskIsDone();
			void JoinWorkersAndShutdown(bool finishTaks = false);

		private:
			void Observer(const TaskQueueEntry& task, TaskEvent event, uint64_t workerIndex, size_t taskId = 0);

			size_t                          m_nextRoundRobinWorkerIndex;    ///< Index of the worker to receive the next requested job, round-robin style.

			std::atomic<unsigned int>       m_tasksRun;                      ///< Counter to track # of jobs run.
			std::atomic<unsigned int>       m_tasksAssisted;                 ///< Counter to track # of jobs run via external Assist*().
			std::atomic<unsigned int>       m_tasksStolen;                   ///< Counter to track # of jobs stolen from another worker's queue.
			std::atomic<affinity_t>         m_usedMask;                     ///< Mask with bits set according to the IDs of the jobs that have executed jobs.
			std::atomic<affinity_t>         m_awokenMask;                   ///< Mask with bits set according to the IDs of the jobs that have been awoken at least once.

			TaskManagerDescriptor             m_desc;                         ///< Descriptor/configuration of the job manager.

			bool                            m_hasPushedJob;                 ///< For profiling - has a job been pushed yet?
			//ProfileClock::time_point        m_firstJobTime;                 ///< For profiling - when was the first job pushed?
			//ProfilingTimeline*				m_timelines;					///< For profiling - a ProfilingTimeline entry for each worker, plus an additional entry to represent the Assist thread.

			std::vector<TaskSystemWorker*>   m_workers;                      ///< Storage for worker instances.

		};
		
		template<size_t MaxTaskNodes = 256>
		class TaskChainBuilder
		{
		public:
			struct Node
			{
				Node() : isGroup(false), groupDependency(nullptr) {}
				~Node() {}

				Node* groupDependency;
				TaskStatePtr	task;
				bool			isGroup;
			};

			Node* AllocNode()
			{
				if (m_nextNodeIndex >= MaxTaskNodes)
					return nullptr;

				Node* node = &m_nodePool[m_nextNodeIndex++];
				*node = Node();

				return node;
			}

			TaskChainBuilder(TaskManager& manager)
				: m_manager(manager)
			{
				Reset();

				m_stack.push_back(AllocNode());
			}

			~TaskChainBuilder()
			{ }

			void Reset()
			{
				m_allJobs.clear();
				m_stack.clear();

				m_last = nullptr;
				m_dependency = nullptr;
				m_nextNodeIndex = 0;
				m_failed = false;
			}

			TaskChainBuilder& Together(char debugChar = 0)
			{
				if (Node* item = AllocNode())
				{
					item->isGroup = true;
					item->groupDependency = m_dependency;

					item->task = m_manager.AddTask([]() {}, debugChar);

					m_allJobs.push_back(item->task);

					m_last = item;
					m_dependency = nullptr;

					m_stack.push_back(item);
				}
				else
				{
					Fail();
				}

				return *this;
			}

			TaskChainBuilder& Do(TaskDelegate delegate, char debugChar = 0)
			{
				Node* owner = m_stack.back();

				if (Node* item = AllocNode())
				{
					item->task = m_manager.AddTask(delegate, debugChar);

					m_allJobs.push_back(item->task);

					if (m_dependency)
					{
						m_dependency->task->AddDependant(item->task);
						m_dependency = nullptr;
					}

					if (owner && owner->isGroup)
					{
						item->task->AddDependant(owner->task);

						if (owner->groupDependency)
						{
							owner->groupDependency->task->AddDependant(item->task);
						}
					}

					m_last = item;
				}
				else
				{
					Fail();
				}

				return *this;
			}

			TaskChainBuilder& Then()
			{
				m_dependency = m_last;
				m_last = (m_dependency) ? m_dependency->groupDependency : nullptr;

				return *this;
			}

			TaskChainBuilder& Close()
			{
				if (!m_stack.empty())
				{
					Node* owner = m_stack.back();
					if (owner->isGroup)
					{
						m_last = owner;
					}
				}

				m_dependency = nullptr;

				if (m_stack.size() > 1)
				{
					m_stack.pop_back();
				}

				return *this;
			}

			TaskChainBuilder& Go()
			{
				if (m_allJobs.empty())
				{
					return *this;
				}

				Then();
				Do([]() {}, 'J');
				m_joinTask = m_allJobs.back();

				for (TaskStatePtr& task : m_allJobs)
				{
					task->SetReady();
				}

				return *this;
			}

			void Fail()
			{
				for (TaskStatePtr& task : m_allJobs)
				{
					task->Cancel();
				}

				m_allJobs.clear();
				m_failed = true;
			}

			void Failed() const
			{
				return m_failed;
			}

			void WaitForAll()
			{
				if (m_joinTask)
				{
					m_joinTask->Wait();
				}
			}

			void AssistAndWaitForAll()
			{
				m_manager.AssistUnitTaskIsDone(m_joinTask);
			}

		private:
			TaskManager&				m_manager;                        ///< Job manager to submit jobs to.

			Node                        m_nodePool[MaxTaskNodes];    ///< Pool of chain nodes (on the stack). The only necessary output of this system is jobs. Nodes are purely internal.
			size_t                      m_nextNodeIndex;            ///< Next free item in the pool.

			std::vector<Node*>          m_stack;                    ///< Internal stack to track groupings.
			std::vector<TaskStatePtr>   m_allJobs;                  ///< All jobs created by the builder, to be readied on completion.

			Node*						m_last;                     ///< Last job to be pushed, to handle setting up dependencies after Then() calls.
			Node*						m_dependency;               ///< Any job promoted to a dependency for the next job, as dicated by Then().

			TaskStatePtr                m_joinTask;                  ///< Final join job that callers can wait on to complete the batch.

			bool                        m_failed;                   ///< Did an error occur during creation of the DAG?
		};
	}
}