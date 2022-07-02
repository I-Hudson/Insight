#pragma once

#include "Core/Defines.h"
#include "Core/TypeAlias.h"

#include <string>
#include <array>
#include <xhash>

namespace Insight
{
	namespace Core
	{
		class IS_CORE GUID
		{
		public:
			GUID() NO_EXPECT;
			GUID(unsigned long data1, unsigned short data2, unsigned short data3, std::array<unsigned char, 8> data4);
			GUID(const GUID& other) NO_EXPECT;
			GUID(GUID&& other) NO_EXPECT;


			GUID& operator=(const GUID& other) NO_EXPECT;
			GUID& operator=(GUID&& other) NO_EXPECT;

			bool operator==(const GUID& other) const;
			bool operator!=(const GUID& other) const;

			void GetNewGUID();
			bool IsValid() const { return *this != s_InvalidGUID; }

			std::string ToString() const;

			static GUID s_InvalidGUID;
		private:
			unsigned long  m_data1;
			unsigned short m_data2;
			unsigned short m_data3;
			unsigned char  m_data4[8];

			template<typename GUID>
			friend struct std::hash;
		};
	}
}

namespace std
{
	template<>
	struct hash<Insight::Core::GUID>
	{
		size_t operator()(const Insight::Core::GUID& guid) const
		{
			size_t hash;
			HashCombine(hash, guid.m_data1);
			HashCombine(hash, guid.m_data2);
			HashCombine(hash, guid.m_data3);
			for (size_t i = 0; i < 8; ++i)
			{
				HashCombine(hash, guid.m_data4[i]);
			}
			return hash;
		}
	};
}