#include "ispch.h"
#include "Engine/Time/Stopwatch.h"

	void Stopwatch::Start()
	{
		m_startTime = std::chrono::high_resolution_clock::now();
		m_endTime = std::chrono::high_resolution_clock::now();
	}

	float Stopwatch::End()
	{
		m_endTime = std::chrono::high_resolution_clock::now();
		return static_cast<float>(Sec());
	}

	double Stopwatch::Sec() const
	{
		return std::chrono::duration<double>(m_endTime - m_startTime).count();
	}

	double Stopwatch::Mill() const
	{
		return std::chrono::duration<double, std::milli>(m_endTime - m_startTime).count();
	}

	u64 Stopwatch::Nano() const
	{
		return static_cast<u64>(std::chrono::duration<double, std::nano>(m_endTime - m_startTime).count());
	}