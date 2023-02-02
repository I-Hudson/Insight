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
			constexpr static u8 c_GUID_BYTE_SIZE = 37;

			GUID() NO_EXPECT;
			GUID(std::nullptr_t);
			GUID(char bytes[c_GUID_BYTE_SIZE]);
			GUID(const GUID& other) NO_EXPECT;
			GUID(GUID&& other) NO_EXPECT;

			GUID& operator=(const GUID& other) NO_EXPECT;
			GUID& operator=(GUID&& other) NO_EXPECT;

			bool operator==(const GUID& other) const;
			bool operator!=(const GUID& other) const;

			void GetNewGUID();
			//// @brief Check if the GUID is a valid.
			bool IsValid() const { return *this != s_InvalidGUID; }

			//// @brief Return a string representation of the GUID.
			std::string ToString() const;

			static GUID s_InvalidGUID;
		private:
			/// @brief Guid is store and formats as a string.
			char m_bytes[c_GUID_BYTE_SIZE];

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
			std::string operator()(Core::GUID const& object)
			{
				std::string str(std::begin(object.m_bytes), std::end(object.m_bytes) - 1);
				return str;
			}
		};
		template<>
		struct PropertyDeserialiser<Core::GUID>
		{
			using InType = std::string;
			using OutType = Core::GUID;
			OutType operator()(InType const& data)
			{
				Core::GUID guid;
				for (size_t i = 0; i < ARRAY_COUNT(guid.m_bytes) - 1; ++i)
				{
					memcpy(&guid.m_bytes[i], &data.at(i), sizeof(Byte));
				}
				return guid;
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
			for (size_t i = 0; i < ARRAY_COUNT(guid.m_bytes); ++i)
			{
				HashCombine(hash, guid.m_bytes[i]);
			}
			return hash;
		}
	};
}