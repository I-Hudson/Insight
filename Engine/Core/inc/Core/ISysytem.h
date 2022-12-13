#pragma once

#include "Core/Defines.h"
#include "Core/TypeAlias.h"
#include "Algorithm/Hash.h"

namespace Insight
{
	namespace Core
	{
		enum class SystemStates
		{
			Not_Initialised,
			Initialised
		};

		/// <summary>
		/// ISystem is defined as something which controls an area/items from a high level.
		/// All systems must be registered with the system register before use.
		/// E.G: Rendering, Resources management, World management, Level partitioning.
		/// </summary>
		class IS_CORE ISystem
		{
		public:
			virtual ~ISystem() { }

			virtual void Initialise() { m_state = SystemStates::Initialised; }
			virtual void Shutdown() { m_state = SystemStates::Not_Initialised; }

			virtual u64 GetSystemId() const = 0;
			SystemStates GetSystemState() const { return m_state; }

		protected:
			SystemStates m_state = SystemStates::Not_Initialised;
		};
	}
}

#define IS_SYSTEM(TypeName) \
	static constexpr u64 s_SystemId = Insight::Algorithm::FNV1a::GetHash64(#TypeName); \
	virtual u64 GetSystemId() const override final { return TypeName::s_SystemId; }