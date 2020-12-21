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

		template<typename T>
		void SetType();

		bool operator==(const Type& rhs) const
		{
			return m_typeId.Hash == rhs.m_typeId.Hash &&
				   m_typeId.Name == rhs.m_typeId.Name &&
				   m_size == rhs.m_size;
		}

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
		m_size = sizeof(T);
	}
}

template<>
struct std::hash<Insight::Type>
{
	size_t operator()(const Insight::Type& type) const
	{
		size_t res = 17;
		res = res * 31 + std::hash<std::string>()(type.GetTypeName());
		res = res * 31 + std::hash<uint64_t>()(type.GetTypeSize());
		return res;
	}
};