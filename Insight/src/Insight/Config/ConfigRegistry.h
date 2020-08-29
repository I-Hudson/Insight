#pragma once

#include "ispch.h"
#include "Insight/Core.h"
#include "Insight/Config/ICVar.h"
#include "Insight/Log.h"

namespace Insight
{
	class IS_API ConfigRegistry
	{
	public:
		static void Register(ICVar* configValue)
		{
			if (Find(configValue->GetKey()) == nullptr)
			{
				m_values.emplace(configValue->GetKey(), configValue);
			}
			else
			{
				IS_CORE_WARN("ConfigRegistry: ConfigValue already exists {0}.", configValue->GetKey());
			}
		}

		static void Unregister(const std::string& key)
		{
			if (Find(key) != nullptr)
			{
				m_values.erase(key);
			}
		}

		static ICVar* Find(const std::string_view& key)
		{
			for (auto v : m_values)
			{
				if (v.first == key)
				{
					return v.second;
				}
			}

			return nullptr;
		}

	private:
		static std::unordered_map<std::string, ICVar*> m_values;
	};
}