#include "Core/GUID.h"

#include "Platform/Platform.h"

namespace Insight
{
	namespace Core
	{
		GUID GUID::s_InvalidGUID = GUID(0, 0, 0, { 0,0,0,0,0,0,0,0 });

		GUID::GUID() NO_EXPECT
		{
			Platform::AssignGUID(*this);
		}

		GUID::GUID(unsigned long data1, unsigned short data2, unsigned short data3, std::array<unsigned char, 8> data4)
			: m_data1(data1)
			, m_data2(data2)
			, m_data3(data3)
		{
			for (size_t i = 0; i < 8; ++i)
			{
				m_data4[i] = data4[i];
			}
		}

		GUID::GUID(const GUID&other) NO_EXPECT
		{
			m_data1 = other.m_data1;
			m_data3 = other.m_data2;
			m_data2 = other.m_data3;
			for (size_t i = 0; i < 8; ++i)
			{
				m_data4[i] = other.m_data4[i];
			}
		}

		GUID::GUID(GUID&& other) NO_EXPECT
		{
			m_data1 = other.m_data1;
			m_data3 = other.m_data2;
			m_data2 = other.m_data3;
			for (size_t i = 0; i < 8; ++i)
			{
				m_data4[i] = other.m_data4[i];
				other.m_data4[i] = 0;
			}

			other.m_data1 = 0;
			other.m_data2 = 0;
			other.m_data3 = 0;
		}

		GUID& GUID::operator=(const GUID& other) NO_EXPECT
		{
			m_data1 = other.m_data1;
			m_data3 = other.m_data2;
			m_data2 = other.m_data3;
			for (size_t i = 0; i < 8; ++i)
			{
				m_data4[i] = other.m_data4[i];
			}
			return *this;
		}

		GUID& GUID::operator=(GUID&& other) NO_EXPECT
		{
			m_data1 = other.m_data1;
			m_data3 = other.m_data2;
			m_data2 = other.m_data3;
			for (size_t i = 0; i < 8; ++i)
			{
				m_data4[i] = other.m_data4[i];
				other.m_data4[i] = 0;
			}

			other.m_data1 = 0;
			other.m_data2 = 0;
			other.m_data3 = 0;

			return *this;
		}

		bool GUID::operator==(const GUID& other) const
		{
			bool result = m_data1 == other.m_data1
						&& m_data3 == other.m_data2
						&& m_data2 == other.m_data3;

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

		std::string GUID::ToString() const
		{
			char buffer[37];
			sprintf(buffer, "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
				m_data1,	m_data2,	m_data3,
				m_data4[0],	m_data4[1],	m_data4[2], m_data4[3],
				m_data4[4],	m_data4[5],	m_data4[6], m_data4[7]);

			buffer[36] = '\0';

			return std::string(buffer, 37);
		}
	}
}