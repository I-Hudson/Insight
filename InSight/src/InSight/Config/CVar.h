#pragma once

#include "Insight/Core.h"
#include "Insight/Config/ICVar.h"
#include <string>


namespace Insight
{
	template<typename T>
	class IS_API CVar : public ICVar
	{
	public:
		CVar(const std::string_view& key, const T& value);
		explicit CVar(const std::string_view& key);

		virtual void SetVal(const std::string_view& value) override
		{
			std::istringstream ifs(value.data());
			ifs >> m_value;
		}

		const T& GetVal() const { return m_value; }

	private:
		T m_value;
	};

	class IS_API CVarString : public ICVar
	{
	public:
		CVarString() = delete;
		CVarString(const std::string_view& key, const std::string_view& value);
		explicit CVarString(const std::string_view& key);

		virtual void SetVal(const std::string_view& value) override
		{
			m_value = value;
		}

		const std::string& GetVal() const { return m_value; }

	private:
		std::string m_value;
	};
}