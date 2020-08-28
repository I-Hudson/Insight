#include "ispch.h"
#include "Insight/Time/Stopwatch.h"

namespace Insight
{
	void Stopwatch::Start()
	{
		m_startTime = std::chrono::high_resolution_clock::now();
		m_endTime = std::chrono::high_resolution_clock::now();
	}

	float Stopwatch::End()
	{
		m_endTime = std::chrono::high_resolution_clock::now();
		return Sec();
	}

	float Stopwatch::Sec() const
	{
		return static_cast<float>(std::chrono::duration<double>(m_endTime - m_startTime).count());
	}

	float Stopwatch::Mill() const
	{
		return static_cast<float>(std::chrono::duration<double, std::milli>(m_endTime - m_startTime).count());
	}

	U64 Stopwatch::Nano() const
	{
		return static_cast<U64>(std::chrono::duration<double, std::nano>(m_endTime - m_startTime).count());

	}
}