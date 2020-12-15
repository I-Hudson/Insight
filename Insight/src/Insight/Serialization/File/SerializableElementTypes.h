#pragma once

#include "Insight/Core.h"
#include <glm/glm.hpp>

namespace Insight
{
	namespace Serialization
	{
		class IS_API SerializableElementType
		{
		public:
			virtual std::string GetValue() = 0;
			std::string GetKey() const { return m_elmentName; }

		protected: 
			std::string m_elmentName;
		};

		/*
		* BASE TYPES
		*/
		class IS_API SerializableElementString : public SerializableElementType
		{
		public:
			virtual std::string GetValue() override;
			void SetKeyData(const std::string& key, const std::string& data);
		private:
			std::string m_data;
		};

		class IS_API SerializableElementInt : public SerializableElementType
		{
		public:
			virtual std::string GetValue() override;
			void SetKeyData(const std::string& key, const I32& data);
		private:
			I32 m_data;
		};

		class IS_API SerializableElementFloat : public SerializableElementType
		{
		public:
			virtual std::string GetValue() override;
			void SetKeyData(const std::string& key, const float& data);
		private:
			float m_data;
		};


		/*
		* VECTORS
		*/
		class IS_API SerializableElementVec2 : public SerializableElementType
		{
		public:
			virtual std::string GetValue() override;
			void SetKeyData(const std::string& key, const glm::vec2& data);
		private:
			glm::vec2 m_data;
		};

		class IS_API SerializableElementVec3 : public SerializableElementType
		{
		public:
			virtual std::string GetValue() override;
			void SetKeyData(const std::string& key, const glm::vec3& data);
		private:
			glm::vec3 m_data;
		};

		class IS_API SerializableElementVec4 : public SerializableElementType
		{
		public:
			virtual std::string GetValue() override;
			void SetKeyData(const std::string& key, const glm::vec4& data);
		private:
			glm::vec4 m_data;
		};

		/*
		* MATRIXS
		*/
		class IS_API SerializableElementMat2 : public SerializableElementType
		{
		public:
			virtual std::string GetValue() override;
			void SetKeyData(const std::string& key, const glm::mat2& data);
		private:
			glm::mat2 m_data;
		};

		class IS_API SerializableElementMat3 : public SerializableElementType
		{
		public:
			virtual std::string GetValue() override;
			void SetKeyData(const std::string& key, const glm::mat3& data);
		private:
			glm::mat3 m_data;
		};

		class IS_API SerializableElementMat4 : public SerializableElementType
		{
		public:
			virtual std::string GetValue() override;
			void SetKeyData(const std::string& key, const glm::mat4& data);
		private:
			glm::mat4 m_data;
		};

	}
}
