#pragma once

#include "Insight/Core.h"
#include "Insight/Library/Library.h"
#include "Insight/Serialization/Serializable.h"

class Model;

namespace Insight
{
	namespace Library
	{
		class ModelLibrary : public Library<Model>
			, public Serialization::Serializable
		{
		public:
			ModelLibrary();
			~ModelLibrary();

			void LoadAssetsFromFolder(const std::string& folderName, const bool& lookInChildren);

			virtual void Serialize(json& data) override;
			virtual void Deserialize(json data) override;

		private:
			const std::vector<std::string> m_extensions =
			{
				".fbx", ".obj",
			};
		};
	}
}