#pragma once

#ifdef IS_MATHS_DIRECTX_MATHS
#include <DirectXMath.h>
#endif
#if defined(IS_MATHS_GLM) || defined(IS_MATHS_CONSTRUCTOR_GLM) || defined(IS_TESTING)
#include <glm/glm.hpp>
#endif 

namespace Insight
{
    namespace Maths
    {
        struct Float2
        {
            union
            {
#ifdef IS_MATHS_DIRECTX_MATHS
                struct { DirectX::XMVECTOR xmvector; };
#endif
#if defined(IS_MATHS_GLM) || defined(IS_MATHS_CONSTRUCTOR_GLM) || defined(IS_TESTING)
                struct { glm::vec2 vec2; };
#endif
                struct { float x, y; };
                struct { float r, g; };
                struct { float data[2]; };
            };
        };
    }
}