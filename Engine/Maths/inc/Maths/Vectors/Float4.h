#pragma once

#ifdef IS_MATHS_DIRECTX_MATHS
#include <DirectXMath.h>
#endif
#if defined(IS_MATHS_GLM) || defined(IS_MATHS_CONSTRUCTOR_GLM) || defined(IS_TESTING)
#include <glm/glm.hpp>
#endif 

#include <type_traits>

namespace Insight
{
    namespace Maths
    {
        template<typename T>
        struct NumberArray4
        {
            static_assert(std::is_arithmetic_v<T>);
            
            union
            {
#ifdef IS_MATHS_DIRECTX_MATHS
                struct { DirectX::XMVECTOR xmvector; };
#endif
#if defined(IS_MATHS_GLM) || defined(IS_MATHS_CONSTRUCTOR_GLM) || defined(IS_TESTING)
                struct { glm::vec<4, T, glm::defaultp> vec4; };
#endif
                struct { T x, y, z, w; };
                struct { T r, g, b, a; };
                struct { T data[4]; };
            };
        };

        using Float4 = NumberArray4<float>;
        using Int4 = NumberArray4<int>;
    }
}