#pragma once

#include "Insight/Core.h"
#include "Insight/InsightAlias.h"
#include <chrono>

namespace Insight
{
	class IS_API Stopwatch
	{
	public:
		Stopwatch() = default;

		void Start();
		float End();

		float Sec() const;
		float Mill() const;
		U64 Nano() const;

	private:
		std::chrono::steady_clock::time_point m_startTime;
		std::chrono::steady_clock::time_point m_endTime;
	};
}