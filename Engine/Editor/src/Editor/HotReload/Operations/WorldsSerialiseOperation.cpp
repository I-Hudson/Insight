#include "Editor/HotReload/Operations/WorldsSerialiseOperation.h"
#include "Editor/HotReload/HotReloadSystem.h"

#include "Serialisation/Serialisers/JsonSerialiser.h"
#include "World/WorldSystem.h"

#include "FileSystem/FileSystem.h"

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

		Serialisation::JsonSerialiser binarySerialiser(false);
		activeWorld->Serialise(&binarySerialiser);

		m_activeWorldSerialisedData = binarySerialiser.GetSerialisedData();
		const Runtime::ProjectInfo& projectInfo = Runtime::ProjectSystem::Instance().GetProjectInfo();
		
		FileSystem::SaveToFile(m_activeWorldSerialisedData, projectInfo.GetIntermediatePath() + "/HotReloadTemp/SerialisedWorld" + Runtime::World::c_FileExtension, true);

		activeWorld->Destroy();
	}

	void WorldsSerialiseOperation::PostLoadOperation()
	{
		TObjectPtr<Runtime::World> activeWorld = Runtime::WorldSystem::Instance().GetActiveWorld();

		if (!m_activeWorldSerialisedData.empty())
		{
			Serialisation::JsonSerialiser binarySerialiser(true);
			binarySerialiser.Deserialise(m_activeWorldSerialisedData);
			activeWorld->Deserialise(&binarySerialiser);
		}

		Reset();
	}
}