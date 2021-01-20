#pragma once

#include "Insight/Core/Core.h"
#include <glm/glm.hpp>

	namespace Serialization
	{
		class IS_API SerializableAttribute
		{
		public:
			std::string GetValue() const { return m_value; }
			std::string GetKey() const { return m_key; }

			void SetKeyValue(const std::string& key, const std::string& value) { m_key = key; m_value = value; }

		private: 
			std::string m_key;
			std::string m_value;
		};
	}