#pragma once

#include "Core/Defines.h"
#include "Core/GUID.h"

#include "Serialisation/Serialiser.h"
#include "Serialisation/ISerialisable.h"

namespace Insight
{
	/// <summary>
	/// Base for all "objects" within the engine.
	/// IObject contains some base level information like type name, type hash.
	/// IObjects are also used with TObjectOPtr and TObjectPtr.
	/// </summary>
	class IS_CORE IObject
	{
	public:
		IObject();
		virtual ~IObject();

		IS_SERIALISABLE_H(IObject);

		void SetGuid(Core::GUID guid);
		Core::GUID GetGuid() const;

	private:
		Core::GUID m_guid;
	};

	OBJECT_SERIALISER(IObject, 1,
		SERIALISE_PROPERTY(Core::GUID, m_guid, 1, 0)
	);
}