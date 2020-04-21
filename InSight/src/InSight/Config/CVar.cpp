#include "ispch.h"
#include "Insight/Config/CVar.h"
#include "ConfigRegistry.h"

namespace Insight
{
	// Declare template classes for CVar.
	template class IS_API CVar<float>;
	template class IS_API CVar<int>;
	std::unordered_map<std::string, ICVar*> ConfigRegistry::m_values;

	template<typename T>
	inline CVar<T>::CVar(const std::string_view& key, const T& value) : ICVar(key), m_value(value)
	{
		ConfigRegistry::Register(this);
	}

	template<typename T>
	inline CVar<T>::CVar(const std::string_view& key) : ICVar(key)
	{
		ConfigRegistry::Register(this);
	}

	CVarString::CVarString(const std::string_view& key, const std::string_view& value) : ICVar(key), m_value(value)
	{
		ConfigRegistry::Register(this);
	}

	CVarString::CVarString(const std::string_view& key) : ICVar(key)
	{
		ConfigRegistry::Register(this);
	}
}