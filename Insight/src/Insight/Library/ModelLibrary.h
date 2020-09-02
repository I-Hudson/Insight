#pragma once

#include "Insight/Core.h"
#include "Insight/Library/Library.h"
#include "Insight/Serialization/Serializable.h"
#include "Insight/Assimp/Model.h"

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

			Model* GetAssetFromPath(const std::string& filePath);
			void LoadAssetsFromFolder(const std::string& folderName, const bool& lookInChildren);

			virtual void Serialize(tinyxml2::XMLNode* out, tinyxml2::XMLDocument* doc, bool force = false) override;
			virtual void Deserialize(tinyxml2::XMLNode* data, bool force = false) override;

		private:
			const std::vector<std::string> m_extensions =
			{
				".fbx", ".obj",
			};
		};
	}
}