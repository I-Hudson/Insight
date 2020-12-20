#pragma once

namespace Insight
{
	class Object;

	struct TypeID
	{
		uint64_t Hash;
		std::string Name;
	};

	class Type
	{
	public:
		const std::string& GetTypeName() const { return m_typeId.Name; }
		const uint64_t& GetTypeSize() const { return m_size; }

	private:
		template<typename T>
		void SetType();

	private:
		TypeID m_typeId;
		uint64_t m_size;

		friend class Object;
	};

	template<typename T>
	inline void Type::SetType()
	{
		// Remove "Class ", from type name;
		std::string typeName = typeid(T).name();
		if (typeName.find("class") != std::string::npos)
		{
			typeName = typeName.substr(6);
			if (size_t offset = typeName.find_last_of(':'))
			{
				typeName = typeName.substr(offset + 1);
			}
		}
		m_typeId.Name = typeName;
		m_typeId.Hash = typeid(T).hash_code();
	}
}