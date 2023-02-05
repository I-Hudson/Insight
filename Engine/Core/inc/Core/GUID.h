#pragma once

#include "Core/Defines.h"
#include "Core/TypeAlias.h"

#include "Serialisation/PropertySerialiser.h"

#include <string>
#include <array>
#include <xhash>

namespace Insight
{
	namespace Core
	{
		/// @brief GUID. Used to store a unique ID which can be compared to other GUIDs.
		class IS_CORE GUID
		{
		public:
			GUID() NO_EXPECT;
			GUID(u32 data1, u16 data2, u16 data3, std::array<u8, 8> data4);
			GUID(u32 data1, u16 data2, u16 data3, u8 data4[8]);
			GUID(const GUID& other) NO_EXPECT;
			GUID(GUID&& other) NO_EXPECT;

			GUID& operator=(const GUID& other) NO_EXPECT;
			GUID& operator=(GUID&& other) NO_EXPECT;

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
			u32 m_data1 = 0;
			u16 m_data2 = 0;
			u16 m_data3 = 0;
			u8  m_data4[8] {0};

			template<typename GUID>
			friend struct std::hash;
			template<typename GUID>
			friend struct Serialisation::PropertySerialiser;
			template<typename GUID>
			friend struct Serialisation::PropertyDeserialiser;
		};
	}

	namespace Serialisation
	{
		template<>
		struct PropertySerialiser<Core::GUID>
		{
			using InType = Core::GUID;
			using OutType = std::string;
			std::string operator()(Core::GUID const& object)
			{
				std::string guidString = object.ToString();
				Core::GUID stringToGuid;
				stringToGuid.StringToGuid(guidString);
				assert(stringToGuid == object);
				return object.ToString();
			}
		};
		template<>
		struct PropertyDeserialiser<Core::GUID>
		{
			using InType = std::string;
			using OutType = Core::GUID;
			OutType operator()(InType const& data)
			{
				OutType result;
				result.StringToGuid(data);
				return result;
			}
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