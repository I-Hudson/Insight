#pragma once

#include "Insight/Core/Core.h"
#include "Insight/Library/Library.h"
#include "Insight/Serialization/Serializable.h"
#include "Insight/Model/Model.h"

class Model;

namespace Insight
{
	namespace Library
	{
		class ModelLibrary : public Library<Model>
			, public TSingleton<ModelLibrary>
			, public Serialization::Serializable
		{
		public:
			ModelLibrary();
			virtual ~ModelLibrary() override;

			SharedPtr<Model> GetAssetFromPath(const std::string& filePath);
			void LoadAssetsFromFolder(const std::string& folderName, const bool& lookInChildren);

			virtual void Serialize(SharedPtr<Insight::Serialization::SerializableElement> element, bool force = false) override;
			virtual void Deserialize(SharedPtr<Insight::Serialization::SerializableElement> element, bool force = false) override;

		private:
			const std::vector<std::string> m_extensions =
			{
				".fbx", ".obj",
			};

			struct ModelLoadThread
			{
				std::vector<std::string> FilesToLoad;
				U32 FileOverallSize = 0;
			};

			std::mutex m_lock;
		};
	}
}