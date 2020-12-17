#pragma once

#include "Insight/Core.h"
#include <glm/glm.hpp>

namespace Insight
{
	namespace Serialization
	{
		enum class SerializableElementDataType : U8
		{
			Unknown = 0,
			String, Int, Float,
			Vec2, Vec3, Vec4,
			Mat2, Mat3, Mat4,

			ENUM_COUNT
		};

		class IS_API SerializableElementType
		{
		public:
			virtual std::string GetValue() = 0;
			virtual void StringToData(const std::string& key, const std::string& dataString) = 0;
			std::string GetKey() const { return m_elmentName; }
			std::string GetDataType() const { return std::to_string((U8)m_dataType); }

		protected: 
			std::string m_elmentName;
			SerializableElementDataType m_dataType;
		};

		/*
		* BASE TYPES
		*/
		class IS_API SerializableElementString : public SerializableElementType
		{
		public:
			SerializableElementString() { m_dataType = SerializableElementDataType::String; }

			virtual std::string GetValue() override;
			virtual void StringToData(const std::string& key, const std::string& dataString) override;
			void SetKeyData(const std::string& key, const std::string& data);
		private:
			std::string m_data;
		};

		class IS_API SerializableElementInt : public SerializableElementType
		{
		public:
			SerializableElementInt() { m_dataType = SerializableElementDataType::Int; }

			virtual std::string GetValue() override;
			virtual void StringToData(const std::string& key, const std::string& dataString) override;
			void SetKeyData(const std::string& key, const I32& data);
		private:
			I32 m_data;
		};

		class IS_API SerializableElementFloat : public SerializableElementType
		{
		public:
			SerializableElementFloat() { m_dataType = SerializableElementDataType::Float; }

			virtual std::string GetValue() override;
			virtual void StringToData(const std::string& key, const std::string& dataString) override;
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
			SerializableElementVec2() { m_dataType = SerializableElementDataType::Vec2; }

			virtual std::string GetValue() override;
			virtual void StringToData(const std::string& key, const std::string& dataString) override;
			void SetKeyData(const std::string& key, const glm::vec2& data);
		private:
			glm::vec2 m_data;
		};

		class IS_API SerializableElementVec3 : public SerializableElementType
		{
		public:
			SerializableElementVec3() { m_dataType = SerializableElementDataType::Vec3; }

			virtual std::string GetValue() override;
			virtual void StringToData(const std::string& key, const std::string& dataString) override;
			void SetKeyData(const std::string& key, const glm::vec3& data);
		private:
			glm::vec3 m_data;
		};

		class IS_API SerializableElementVec4 : public SerializableElementType
		{
		public:
			SerializableElementVec4() { m_dataType = SerializableElementDataType::Vec4; }

			virtual std::string GetValue() override;
			virtual void StringToData(const std::string& key, const std::string& dataString) override;
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
			SerializableElementMat2() { m_dataType = SerializableElementDataType::Mat2; }

			virtual std::string GetValue() override;
			virtual void StringToData(const std::string& key, const std::string& dataString) override;
			void SetKeyData(const std::string& key, const glm::mat2& data);
		private:
			glm::mat2 m_data;
		};

		class IS_API SerializableElementMat3 : public SerializableElementType
		{
		public:
			SerializableElementMat3() { m_dataType = SerializableElementDataType::Mat3; }

			virtual std::string GetValue() override;
			virtual void StringToData(const std::string& key, const std::string& dataString) override;
			void SetKeyData(const std::string& key, const glm::mat3& data);
		private:
			glm::mat3 m_data;
		};

		class IS_API SerializableElementMat4 : public SerializableElementType
		{
		public:
			SerializableElementMat4() { m_dataType = SerializableElementDataType::Mat4; }

			virtual std::string GetValue() override;
			virtual void StringToData(const std::string& key, const std::string& dataString) override;
			void SetKeyData(const std::string& key, const glm::mat4& data);
		private:
			glm::mat4 m_data;
		};

	}
}
