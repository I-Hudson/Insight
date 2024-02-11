#pragma once

#ifdef IS_RESOURCE_HANDLES_ENABLED

#include "Core/TypeAlias.h"
#include "Graphics/Defines.h"

namespace Insight
{
    namespace Graphics
    {
        struct Texture;
        template<typename, typename, u64>
        class RHI_ResourcePool;

        template<typename T>
        constexpr bool HandleAllowedTypes =
            std::is_same_v<T, Texture>;

        template<typename T = void>
        struct IS_GRAPHICS RHI_Handle
        {
            static_assert(HandleAllowedTypes<T> == true, "RHI_Handle must be 'typedef' with a valid 'T' type before use.");
        protected:
            u32 Index;
            u32 Generation;

            template<typename, typename, u64>
            friend class RHI_ResourcePool;
        };
    }
}
#endif