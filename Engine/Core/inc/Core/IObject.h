#pragma once

#include "Core/Defines.h"
#include "Core/GUID.h"

#include "Serialisation/Serialiser.h"
#include "Serialisation/ISerialisable.h"

#include <string>

#define IS_OBJECT(TYPE)\
virtual const char* GetTypeName() const override { return STRINGIZE_NX(TYPE); }


namespace Insight
{
	class ObjectManager;

	/// <summary>
	/// Base for all "objects" within the engine.
	/// IObject contains some base level information like type name, type hash.
	/// IObjects are also used with TObjectOPtr and TObjectPtr.
	/// </summary>
	class IS_CORE IObject : public Serialisation::ISerialisable
	{
	public:
		IObject();
		virtual ~IObject();

		IS_SERIALISABLE_H(IObject);

		void SetGuid(const Core::GUID& guid);
		Core::GUID GetGuid() const;
		virtual const char* GetTypeName() const { return ""; }

	private:
		Core::GUID m_guid;
		i64 m_objectIndex = -1;

		friend class ObjectManager;
	};

	namespace Serialisation
	{
		struct IObjectGuid {};
		template<>
		struct ComplexSerialiser<IObjectGuid, Core::GUID, IObject>
		{
			void operator()(ISerialiser* serialiser, Core::GUID& guid, IObject* iobject) const
			{
				if (serialiser->IsReadMode())
				{
					std::string guidStr;
					serialiser->Read("m_guid", guidStr);

					PropertyDeserialiser<Core::GUID> guidDeserialiser;
					iobject->SetGuid(guidDeserialiser(guidStr));
				}
				else
				{
					PropertySerialiser<Core::GUID> guidSerialiser;
					serialiser->Write("m_guid", guidSerialiser(guid));
				}
			}
		};
	}

	OBJECT_SERIALISER(IObject, 1,
		SERIALISE_COMPLEX(Serialisation::IObjectGuid, m_guid, 1, 0)
	);
}