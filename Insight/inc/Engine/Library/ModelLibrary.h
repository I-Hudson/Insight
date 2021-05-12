#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Library/Library.h"
#include "Engine/Serialization/Serializable.h"
#include "Engine/Model/Model.h"

class Model;

	namespace Library
	{
		class ModelLibrary : public Library<Model>
			, public TSingleton<ModelLibrary>
			, public Serialization::Serializable
		{
		public:
			ModelLibrary();
			virtual ~ModelLibrary() override;

			Model* GetAssetFromPath(const std::string& filePath);
			void LoadAssetsFromFolder(const std::string& folderName, const bool& lookInChildren);

			virtual void Serialize(Serialization::SerializableElement* element, bool force = false) override;
			virtual void Deserialize(Serialization::SerializableElement* element, bool force = false) override;

		private:
			const std::vector<std::string> m_extensions =
			{
				".fbx", ".obj",
			};

			struct ModelLoadThread
			{
				std::vector<std::string> FilesToLoad;
				u32 FileOverallSize = 0;
			};

			std::mutex m_lock;
		};
	}