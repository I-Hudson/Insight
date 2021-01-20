#pragma once

#include "Insight/Core/Core.h"
#include <string>

	class IS_API ICVar
	{
	public:
		virtual void SetVal(const std::string_view& value) = 0;
		const std::string GetKey() const { return m_key; }

	protected:
		ICVar(const std::string_view& key)
			: m_key(key)
		{ }

		virtual ~ICVar() { }

	private:
		std::string m_key;
	};