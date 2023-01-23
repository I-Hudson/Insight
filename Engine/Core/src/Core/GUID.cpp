#include "Core/GUID.h"

#include "Platforms/Platform.h"

namespace Insight
{
	namespace Core
	{
		GUID GUID::s_InvalidGUID = GUID(nullptr);

		GUID::GUID() NO_EXPECT
		{
			Platform::AssignGUID(*this);
		}

		Core::GUID::GUID(std::nullptr_t)
		{
			Platform::MemSet(m_bytes, 0, c_GUID_BYTE_SIZE);
		}

		GUID::GUID(char bytes[c_GUID_BYTE_SIZE])
			//: m_data1(data1)
			//, m_data2(data2)
			//, m_data3(data3)
		{
			ASSERT(bytes != nullptr);
			Platform::MemCopy(m_bytes, bytes, c_GUID_BYTE_SIZE);
		}

		GUID::GUID(const GUID& other) NO_EXPECT
		{
			Platform::MemCopy(m_bytes, other.m_bytes, c_GUID_BYTE_SIZE);
		}

		GUID::GUID(GUID&& other) NO_EXPECT
		{
			Platform::MemCopy(m_bytes, other.m_bytes, c_GUID_BYTE_SIZE);
			Platform::MemSet(other.m_bytes, 0, c_GUID_BYTE_SIZE);
		}

		GUID& GUID::operator=(const GUID& other) NO_EXPECT
		{
			Platform::MemCopy(m_bytes, other.m_bytes, c_GUID_BYTE_SIZE);
			return *this;
		}

		GUID& GUID::operator=(GUID&& other) NO_EXPECT
		{
			Platform::MemCopy(m_bytes, other.m_bytes, c_GUID_BYTE_SIZE);
			Platform::MemSet(other.m_bytes, 0, c_GUID_BYTE_SIZE);

			return *this;
		}

		bool GUID::operator==(const GUID& other) const
		{
			return Platform::MemCompare(m_bytes, other.m_bytes, c_GUID_BYTE_SIZE);
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
			return std::string(std::begin(m_bytes), std::end(m_bytes));
		}
	}
}