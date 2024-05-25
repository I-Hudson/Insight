#include "Threading/Task.h"

namespace Insight
{
	namespace Threading
	{
		Task::Task()
		{ }

		Task::Task(ITaskFuncWrapper* funcWrapper)
			: m_functionWrapper(std::move(funcWrapper))
			, m_state(TaskStates::Queued)
		{
		}

		Task::~Task()
		{
			Delete(m_functionWrapper);
		}

		void Task::Wait()
		{
			std::unique_lock lock(m_mutex);
			if (!IsFinished() || !IsCancled())
			{
				m_cv.wait(lock, [this]()
					{
							return m_state == TaskStates::Finished
								|| m_state == TaskStates::Canceled;
					});
			}
		}

		void Task::Call()
		{
			{
				std::lock_guard lock(m_mutex);
				m_state = TaskStates::Running;
				m_functionWrapper->Call();
				m_state = TaskStates::Finished;
			}
			m_cv.notify_all();
		}
	}
}