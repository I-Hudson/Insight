#pragma once

#include "Threading/TaskResult.h"
#include "Core/Memory.h"

#include <tuple>

namespace Insight
{
	namespace Threading
	{
		class ITaskFuncWrapper
		{
		public:
			virtual ~ITaskFuncWrapper() = default;
			virtual void Call() = 0;
		};

		template <typename ResultType, typename Func, typename... Args>
		class TaskFuncWrapper : public ITaskFuncWrapper
		{
		public:
			TaskFuncWrapper(TaskResult<ResultType>* taskResult, Func func, Args... args)
				: m_taskResult(taskResult)
				, m_func(func)
				, m_args(std::move(args)...)
			{ }

			virtual ~TaskFuncWrapper()
			{ }

			virtual void Call() override
			{
				if constexpr (std::is_void_v<ResultType>)
				{
					std::apply(m_func, m_args);
				}
				else
				{
					m_taskResult->SetResult(std::apply(m_func, m_args));
				}
			}

		private:
			TaskResult<ResultType>* m_taskResult;
			Func m_func;
			std::tuple<Args...> m_args;
		};
	}
}