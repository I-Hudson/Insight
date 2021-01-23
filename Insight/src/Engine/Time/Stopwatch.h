#pragma once

#include "Engine/Core/Core.h"
#include <chrono>

	class IS_API Stopwatch
	{
	public:
		Stopwatch() = default;

		void Start();
		float End();

		double Sec() const;
		double Mill() const;
		U64 Nano() const;

	private:
		std::chrono::steady_clock::time_point m_startTime;
		std::chrono::steady_clock::time_point m_endTime;
	};