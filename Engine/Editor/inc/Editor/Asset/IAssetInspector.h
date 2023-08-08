#pragma once

#include "Editor/Defines.h"

#include "Core/Singleton.h"
#include "Core/Memory.h"

#include <Reflect.h>

#include <vector>
#include <string>

#include "Generated/IAssetInspector_reflect_generated.h"

namespace Insight
{
	namespace Runtime
	{
		class AssetInfo;
	}

	namespace Editor
	{
		/// @brief Interface for all asset inspectors to inherit from. Used to draw information about an asset in the 
		/// inspector window.
		REFLECT_CLASS()
		class IS_EDITOR IAssetInspector
		{
			REFLECT_GENERATED_BODY();
		public:
			IAssetInspector(std::vector<std::string> extensions);
			virtual ~IAssetInspector();

			virtual void Draw(const Runtime::AssetInfo* assetInfo) = 0;

			bool HasExtension(std::string_view extension) const;
			bool HasAnyExtension(std::vector<std::string_view> extensions) const;
			bool HasAnyExtension(std::vector<std::string> extensions) const;

			const std::vector<std::string>& GetExtensions() const;

		private:
			std::vector<std::string> m_extensions;
		};

		class IS_EDITOR AssetInspectorRegistry : public Core::Singleton<AssetInspectorRegistry>
		{
		public:
			AssetInspectorRegistry();
			~AssetInspectorRegistry();

			void Initialise();
			void Shutdown();

			template<typename T>
			void RegisterInspector()
			{
				static_assert(std::is_base_of_v<IAssetInspector, T>);
				m_inspectors.push_back(New<T>());
			}

			IAssetInspector* GetInspectorFromExtension(std::string_view extension) const;

		private:
			std::vector<IAssetInspector*> m_inspectors;
		};
	}
}