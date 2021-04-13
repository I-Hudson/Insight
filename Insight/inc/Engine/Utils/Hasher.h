#pragma once

#include "Engine/Core/Common.h"

namespace Insight::Utils
{
	class Hasher
	{
		Hasher()
			: m_hash(0)
		{ }
		~Hasher()
		{ }

		template<typename T>
		INLINE u64 Hash(T& v)
		{
			std::hash<T> hasher;
			m_hash ^= hasher(v) + 0x9e3779b9 + (m_hash << 6) + (m_hash >> 2);
		}

	private:
		u64 m_hash;
	};
}