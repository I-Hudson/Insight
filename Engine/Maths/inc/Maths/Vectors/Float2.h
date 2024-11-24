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
        /*
        template<typename T>
        struct NumberArray2
        {
            static_assert(std::is_arithmetic_v<T>);

            union
            {
#ifdef IS_MATHS_DIRECTX_MATHS
                struct { DirectX::XMVECTOR xmvector; };
#endif
#if defined(IS_MATHS_GLM) || defined(IS_MATHS_CONSTRUCTOR_GLM) || defined(IS_TESTING)
                struct { glm::vec<2, T, glm::defaultp> vec2; };
#endif
                struct { T x, y; };
                struct { T r, g; };
                struct { T data[2]; };
            };
        };

        using Float2 = NumberArray2<float>;
        using Int2 = NumberArray2<int>;
        */
    }
}