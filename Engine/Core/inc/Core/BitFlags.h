#pragma once

#include "Core/TypeAlias.h"
#include <typeinfo>

namespace Insight
{
	template<typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
	class BitFlags
	{
	public:
		BitFlags() { }
		BitFlags(const T flagBit) : m_flags(static_cast<u64>(flagBit)) { }

		u64 Data() const { return m_flags; }

		bool operator==(const BitFlags other) const { return m_flags == other.m_flags; }
		bool operator!=(const BitFlags other) const { return !(*this == other) }

		BitFlags operator|(const T flagBit) { return BitFlags(m_flags).Set(flagBit); }
		BitFlags operator&(const T flagBit) { return BitFlags(m_flags).Unset(flagBit); }

		BitFlags& operator|=(const T flagBit) { Set(flagBit); return *this; }
		BitFlags& operator&=(const T flagBit) { Unset(flagBit); return *this; }

		void Set(const T flagBit)
		{
			ASSERT(static_cast<u64>(flagBit) <= (1ull << 63));
			m_flags |= static_cast<u64>(flagBit);
		}

		void Unset(const T flagBit)
		{
			ASSERT(static_cast<u64>(flagBit) <= (1ull << 63));
			m_flags &= ~static_cast<u64>(flagBit);
		}

	private:
		u64 m_flags = 0;
	};
}