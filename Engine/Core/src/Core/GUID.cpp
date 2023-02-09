#include "Core/GUID.h"

#include "Platforms/Platform.h"

namespace Insight
{
	namespace Core
	{
		GUID GUID::s_InvalidGUID = GUID(0, 0, 0, { 0,0,0,0,0,0,0,0 });

		GUID::GUID() NO_EXPECT
		{
			Platform::AssignGUID(*this);
		}

		GUID::GUID(u32 data1, u16 data2, u16 data3, std::array<u8, 8> data4)
			: m_data1(data1)
			, m_data2(data2)
			, m_data3(data3)
		{
			for (size_t i = 0; i < 8; ++i)
			{
				m_data4[i] = data4[i];
			}
		}

		GUID::GUID(u32 data1, u16 data2, u16 data3, u8 data4[8])
			: m_data1(data1)
			, m_data2(data2)
			, m_data3(data3)
		{
			for (size_t i = 0; i < 8; ++i)
			{
				m_data4[i] = data4[i];
			}
		}

		GUID::GUID(const GUID& other) NO_EXPECT
		{
			m_data1 = other.m_data1;
			m_data2 = other.m_data2;
			m_data3 = other.m_data3;
			for (size_t i = 0; i < 8; ++i)
			{
				m_data4[i] = other.m_data4[i];
			}
		}

		GUID::GUID(GUID&& other) NO_EXPECT
		{
			m_data1 = other.m_data1;
			m_data2 = other.m_data2;
			m_data3 = other.m_data3;
			for (size_t i = 0; i < 8; ++i)
			{
				m_data4[i] = other.m_data4[i];
			}

			other.m_data1 = 0;
			other.m_data2 = 0;
			other.m_data3 = 0;
			Platform::MemSet(other.m_data4, 0, sizeof(other.m_data4));
		}

		GUID& GUID::operator=(const GUID& other) NO_EXPECT
		{
			m_data1 = other.m_data1;
			m_data2 = other.m_data2;
			m_data3 = other.m_data3;
			for (size_t i = 0; i < 8; ++i)
			{
				m_data4[i] = other.m_data4[i];
			}
			return *this;
		}

		GUID& GUID::operator=(GUID&& other) NO_EXPECT
		{
			m_data1 = other.m_data1;
			m_data2 = other.m_data2;
			m_data3 = other.m_data3;
			for (size_t i = 0; i < 8; ++i)
			{
				m_data4[i] = other.m_data4[i];
				other.m_data4[i] = 0;
			}

			other.m_data1 = 0;
			other.m_data2 = 0;
			other.m_data3 = 0;
			Platform::MemSet(other.m_data4, 0, sizeof(other.m_data4));

			return *this;
		}

		bool GUID::operator==(const GUID& other) const
		{
			bool result = m_data1 == other.m_data1
				&& m_data2 == other.m_data2
				&& m_data3 == other.m_data3;

			for (size_t i = 0; i < 8; ++i)
			{
				result &= m_data4[i] == other.m_data4[i];
			}

			return result;
		}

		bool Insight::Core::GUID::operator!=(const GUID& other) const
		{
			return !(*this == other);
		}

		void Insight::Core::GUID::GetNewGUID()
		{
			Platform::AssignGUID(*this);
		}

		void Insight::Core::GUID::StringToGuid(std::string_view string)
		{
			const auto ret = sscanf(string.data(), "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
				&m_data1, &m_data2, &m_data3,
				&m_data4[0], &m_data4[1], &m_data4[2], &m_data4[3],
				&m_data4[4], &m_data4[5], &m_data4[6], &m_data4[7]);
			if (ret != 11)
			{
				throw std::logic_error("Unvalid GUID, format should be {00000000-0000-0000-0000-000000000000}");
			}
		}

		std::string GUID::ToString() const
		{
			std::string buffer;
			buffer.resize(36);
			sprintf(buffer.data(), "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
				m_data1, m_data2, m_data3,
				m_data4[0], m_data4[1], m_data4[2], m_data4[3],
				m_data4[4], m_data4[5], m_data4[6], m_data4[7]);
			return buffer;
		}
	}
}