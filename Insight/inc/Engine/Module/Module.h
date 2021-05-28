#pragma once

#include "Engine/Core/Core.h"
#include <vector>

namespace Insight::Module
{
	class ModuleManager;

	enum class ModulePriority
	{
		LOW,
		MEMDIUM,
		HIGH
	};

	enum class ModuleState
	{
		Init,
		Loading,
		Running,
		Unloading,
		Deinit
	};

	class Module
	{
	public:
		Module() : m_state(ModuleState::Init), m_modulePriority(ModulePriority::MEMDIUM) { }
		virtual ~Module() { }

		virtual void OnCreate() { }
		virtual void Update(const float& deltaTime) = 0;

		const bool ShouldManuallUpate() const { return m_manuallUpdate; }
		void SetManuallyUpdate(const bool& state) { m_manuallUpdate = state; }
		const bool& ShouldDestroyManually() const { return m_shouldDestroyManually; }
		void SetDestroyManually(const bool& state) { m_shouldDestroyManually = state; }
		const ModuleState& GetState() const { return m_state; }

	protected:
		bool m_manuallUpdate = false;
		bool m_shouldDestroyManually = false;
		ModuleState m_state;
		ModulePriority m_modulePriority;

		friend Module;
		friend ModuleManager;
	};
}