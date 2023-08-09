#include "Editor/Asset/IAssetInspector.h"
#include "Editor/Asset/Inspector/AssetPackageAssetInspector.h"
#include "Editor/Asset/Inspector/TextureAssetInspector.h"

#include "Algorithm/Vector.h"

namespace Insight
{
	namespace Editor
	{
		IAssetInspector::IAssetInspector(std::vector<std::string> extensions)
			: m_extensions(std::move(extensions))
		{
		}

		IAssetInspector::~IAssetInspector()
		{ }

		bool IAssetInspector::HasExtension(std::string_view extension) const
		{
			return Algorithm::VectorContainsIf(m_extensions, [&extension](const std::string& str)
				{
					return str == extension;
				});
		}

		bool IAssetInspector::HasAnyExtension(std::vector<std::string_view> extensions) const
		{
			for (std::string_view view : extensions)
			{
				if (HasExtension(view))
				{
					return true;
				}
			}
			return false;
		}

		bool IAssetInspector::HasAnyExtension(std::vector<std::string> extensions) const
		{
			std::vector<std::string_view> extensionsView(extensions.begin(), extensions.end());
			return HasAnyExtension(extensionsView);
		}

		const std::vector<std::string>& IAssetInspector::GetExtensions() const
		{
			return m_extensions;
		}

		//----------------------------------------------
		// AssetInspectorRegistry
		//----------------------------------------------
		AssetInspectorRegistry::AssetInspectorRegistry()
		{
		}

		AssetInspectorRegistry::~AssetInspectorRegistry()
		{
			Shutdown();
		}

		void AssetInspectorRegistry::Initialise()
		{
			RegisterInspector<TextureAssetInspector>();
			RegisterInspector<AssetPackageAssetInspector>();
		}

		void AssetInspectorRegistry::Shutdown()
		{
			for (IAssetInspector*& inspector : m_inspectors)
			{
				Delete(inspector);
			}
		}

		IAssetInspector* AssetInspectorRegistry::GetInspectorFromExtension(std::string_view extension) const
		{
			for (IAssetInspector* inspector : m_inspectors)
			{
				if (inspector->HasExtension(extension))
				{
					return inspector;
				}
			}
			return nullptr;
		}
	}
}