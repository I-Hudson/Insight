#include "Asset/Importers/ShaderImporter.h"
#include "Asset/Assets/Shader.h"

#include "Graphics/RenderContext.h"

namespace Insight
{
    namespace Runtime
    {
        ShaderImporter::ShaderImporter(std::vector<const char*> validFileExtensions)
        { }

        ShaderImporter::~ShaderImporter()
        { }

        Ref<Asset> ShaderImporter::CreateAsset(const AssetInfo* assetInfo) const
        {
            return Ref<Asset>(::New<ShaderAsset>(assetInfo));
        }

        void ShaderImporter::Import(Ref<Asset>& asset, const AssetInfo* assetInfo, const std::string_view path) const
        {
            Ref<ShaderAsset> shader = asset.As<ShaderAsset>();

            Graphics::ShaderDesc shaderDesc;
            shaderDesc.MainFunc = "Main";

            Graphics::RenderContext::Instance().GetShaderManager().GetOrCreateShader(shaderDesc);

            shader->m_name = ;
        }
    }
}