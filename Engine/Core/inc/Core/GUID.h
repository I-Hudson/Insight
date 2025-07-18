#pragma once

#include "Core/Defines.h"
#include "Core/TypeAlias.h"

#include <string>
#include <array>

namespace Insight
{
	namespace Core
	{
		/// @brief GUID. Used to store a unique ID which can be compared to other GUIDs.
		class IS_CORE GUID
		{
		public:
			GUID();
			GUID(u32 data1, u16 data2, u16 data3, std::array<u8, 8> data4);
			GUID(u32 data1, u16 data2, u16 data3, u8 data4[8]);
			GUID(const GUID& other);
			GUID(GUID&& other);

			GUID& operator=(const GUID& other);
			GUID& operator=(GUID&& other);

			bool operator==(const GUID& other) const;
			bool operator!=(const GUID& other) const;

			void GetNewGUID();
			//// @brief Check if the GUID is a valid.
			bool IsValid() const { return *this != s_InvalidGUID; }

			void StringToGuid(std::string_view string);
			//// @brief Return a string representation of the GUID.
			std::string ToString() const;

			static GUID s_InvalidGUID;
		private:
			unsigned long m_data1 = 0;
			unsigned short m_data2 = 0;
			unsigned short m_data3 = 0;
			unsigned char  m_data4[8] {0};

			template<typename>
			friend struct std::hash;
		};
	}
}

namespace std
{
	//// <summary>
	//// Custom std::has function for GUID struct.
	//// </summary>
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