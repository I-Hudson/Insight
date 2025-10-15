#include "Editor/HotReload/Operations/WorldsSerialiseOperation.h"
#include "Editor/HotReload/HotReloadSystem.h"

#include "Serialisation/Serialisers/BinarySerialiser.h"
#include "World/WorldSystem.h"


namespace Insight::Editor
{
	WorldsSerialiseOperation::WorldsSerialiseOperation()
	{ }

	WorldsSerialiseOperation::~WorldsSerialiseOperation()
	{ }

	void WorldsSerialiseOperation::Reset()
	{ 
		m_activeWorldSerialisedData.clear();
	}

	void WorldsSerialiseOperation::PreUnloadOperation()
	{
		TObjectPtr<Runtime::World> activeWorld = Runtime::WorldSystem::Instance().GetActiveWorld();

		Serialisation::BinarySerialiser binarySerialiser(false);
		activeWorld->Serialise(&binarySerialiser);

		m_activeWorldSerialisedData = binarySerialiser.GetSerialisedData();

		activeWorld->Destroy();
	}

	void WorldsSerialiseOperation::PostLoadOperation()
	{
		TObjectPtr<Runtime::World> activeWorld = Runtime::WorldSystem::Instance().GetActiveWorld();

		Serialisation::BinarySerialiser binarySerialiser(true);
		binarySerialiser.Deserialise(m_activeWorldSerialisedData);
		Reset();
		activeWorld->Deserialise(&binarySerialiser);
	}
}