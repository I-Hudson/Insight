#include "Asset/Assets/Shader.h"

#include "Graphics/RenderContext.h"

namespace Insight
{
    namespace Runtime
    {
        ShaderAsset::ShaderAsset(const AssetInfo* assetInfo)
            : Asset(assetInfo)
        { }

        ShaderAsset::~ShaderAsset()
        { }

        std::string_view ShaderAsset::GetName() const
        {
            return m_name;
        }

        bool ShaderAsset::IsShaderStages(const Graphics::ShaderStageFlags shaderStages) const
        {
            if (shaderStages & Graphics::ShaderStageFlagBits::ShaderStage_Vertex) { return true; }
            if (shaderStages & Graphics::ShaderStageFlagBits::ShaderStage_TessControl) { return true; }
            if (shaderStages & Graphics::ShaderStageFlagBits::ShaderStage_TessEval) { return true; }
            if (shaderStages & Graphics::ShaderStageFlagBits::ShaderStage_Geometry) { return true; }
            if (shaderStages & Graphics::ShaderStageFlagBits::ShaderStage_Pixel) { return true; }
            return false;
        }

        Graphics::ShaderStageFlags ShaderAsset::GetShaderStages() const
        {
            return m_shaderStages;
        }

        void ShaderAsset::Reload()
        {
            Graphics::RenderContext::Instance().GetShaderManager().DestroyShader(m_rhiShader);
            m_rhiShader = Graphics::RenderContext::Instance().GetShaderManager().GetOrCreateShader(shaderDesc);
        }

        void ShaderAsset::OnUnload()
        {
        }
    }
}