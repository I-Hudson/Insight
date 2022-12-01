#pragma once

#include "Core/Defines.h"
#include "Platform/Platform.h"

namespace Insight
{
	namespace Threading
	{
		template<typename ResultType, bool>
		class ITaskResult
		{
		public:
			ITaskResult() = default;
			~ITaskResult() = default;

			void GetResult() { FAIL_ASSERT_MSG("[ITaskResult::GetResult] CAn't call 'GetResult' on 'void' result."); }
			//void SetResult(ResultType result) { }

		private:
		};

		template<typename ResultType>
		class ITaskResult<ResultType, false>
		{
		public:
			ITaskResult() = default;
			~ITaskResult() = default;

			ResultType& GetResult() { return m_result; }
			void SetResult(ResultType result) { m_result = result; m_isReady = true; }

		private:
			ResultType m_result;
			bool m_isReady = false;
		};

		template<typename ResultType>
		using TaskResult = ITaskResult<ResultType, std::is_void_v<ResultType>>;
	}
}