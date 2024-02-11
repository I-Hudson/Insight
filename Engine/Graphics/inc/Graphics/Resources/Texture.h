#pragma once
#ifdef IS_RESOURCE_HANDLES_ENABLED

#include "Graphics/Enums.h"
#include "Graphics/RHI/RHI_Handle.h"

namespace Insight
{
    namespace Graphics
    {
        struct TextureDrawData
        {
            float ClearColour[4] = { 0,0,0,1 };
            u32 LayerCount = 1;
        };

        /// @brief POD struct for texture information.
        struct Texture
        {
            std::string DebugName;
            TextureType TextureType;
            int Width, Height, Depth = -1;
            ImageLayout Layout;
            PixelFormat Format;
            ImageUsageFlags ImageUsage = 0;
            u32 MipCount = 1;
            u32 LayerCount = 1;

            float ClearColour[4] = { 0,0,0,1 };
        };

        ///======================================================
        /// GRAPHICS API
        ///======================================================
    
    }
}
#ifdef IS_DX12_ENABLED
namespace D3D12MA
{
    class Allocation;
}

namespace Insight
{
    namespace Graphics
    {
        struct TextureDrawData_DX12 : public TextureDrawData
        {
            D3D12MA::Allocation* D3D12Allocation = nullptr;
        };
    }
}
#endif
#endif