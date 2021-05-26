#pragma once

#include "Engine/Core/Core.h"
#include <vector>

namespace Module
{
	class ModuleManager;

	enum ModulePriority
	{
		LOW,
		MEMDIUM,
		HIGH
	};

	class IS_API Module
	{
	public:
		virtual ~Module()
		{ }

		virtual void OnCreate() { }
		virtual void Update(const float& deltaTime) = 0;

		const bool ShouldManuallUpate() const { return m_manuallUpdate; }
		void SetManuallyUpdate(const bool& state) { m_manuallUpdate = state; }
		const bool& ShouldDestroyManually() const { return m_shouldDestroyManually; }
		void SetDestroyManually(const bool& state) { m_shouldDestroyManually = state; }

	protected:
		bool m_manuallUpdate = false;
		bool m_shouldDestroyManually = false;
		ModulePriority m_modulePriority;

		friend Module;
		friend ModuleManager;
	};
}