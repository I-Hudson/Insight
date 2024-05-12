#pragma once

#include "Runtime/Defines.h"
#include "Asset/Asset.h"

#include "Graphics/Enums.h"
#include "Graphics/ShaderDesc.h"

#include "Generated/Shader_reflect_generated.h"

namespace Insight
{
    namespace Graphics
    {
        class RHI_Shader;
    }

    namespace Runtime
    {
        class ShaderImporter;

        REFLECT_CLASS();
        class IS_RUNTIME ShaderAsset : public Asset
        {
            REFLECT_GENERATED_BODY();
        public:
            ShaderAsset(const AssetInfo* assetInfo);
            virtual ~ShaderAsset() override;

            std::string_view GetName() const;

            bool IsShaderStages(const Graphics::ShaderStageFlags) const;
            Graphics::ShaderStageFlags GetShaderStages() const;

            void Reload();

        private:
            std::string m_name;
            Graphics::ShaderStageFlags m_shaderStages;
            Graphics::ShaderDesc shaderDesc;
            Graphics::RHI_Shader* m_rhiShader;

            friend class ShaderImporter;
        };
    }
}