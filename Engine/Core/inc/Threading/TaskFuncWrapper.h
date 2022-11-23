#pragma once

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

		template <typename Func, typename... Args>
		class TaskFuncWrapper : public ITaskFuncWrapper
		{
		public:
			TaskFuncWrapper(Func func, Args... args)
				: m_func(func)
				, m_args(std::move(args)...)
			{ }

			virtual ~TaskFuncWrapper()
			{ }

			virtual void Call() override
			{
				std::apply(m_func, m_args);
			}

		private:
			Func m_func;
			std::tuple<Args...> m_args;
		};
	}
}