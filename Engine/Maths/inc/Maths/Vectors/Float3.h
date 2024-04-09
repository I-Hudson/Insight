#pragma once

#ifdef IS_MATHS_DIRECTX_MATHS
#include <DirectXMath.h>
#endif
#if defined(IS_MATHS_GLM) || defined(IS_TESTING)
#include <glm/glm.hpp>
#endif 

namespace Insight
{
    namespace Maths
    {
        struct Float3
        {
            union
            {
#ifdef IS_MATHS_DIRECTX_MATHS
                struct { DirectX::XMVECTOR xmvector; };
#endif
#if defined(IS_MATHS_GLM) || defined(IS_TESTING)
                struct { glm::vec3 vec3; };
#endif
                struct { float x, y, z; };
                struct { float r, g, b; };
                struct { float data[3]; };
            };
        };
    }
}