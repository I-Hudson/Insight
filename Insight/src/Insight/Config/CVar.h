#pragma once

#include "Insight/Core/Core.h"
#include "Insight/Config/ICVar.h"
#include "Insight/Config/CVarParser.h"

	template<typename T>
	class IS_API CVar : public ICVar
	{
	public:
		CVar(const std::string_view& key, const T& value);
		explicit CVar(const std::string_view& key);

		virtual void SetVal(const std::string_view& value) override
		{
			/*if (value.find_first_of("array") != std::string::npos)
			{
				int valueStringIndex = value.find_first_of("array") + 2;
				std::string_view valueString = value.substr(valueStringIndex);
				std::vector<std::string> values;
				char c = valueString[valueStringIndex];

				while (c != ')')
				{
					if (c == ',')
					{
						values.push_back(valueString[valueStringIndex - 1]);
					}
					++valueStringIndex;
				}

				for (size_t i = 0; i < values.size(); ++i)
				{
					std::istringstream ifs(values[i].data());
					ifs >> m_value[i];
				}
			}
			else
			{*/
			CVarParser<T> parser;
			m_value = parser.Parse(value);
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