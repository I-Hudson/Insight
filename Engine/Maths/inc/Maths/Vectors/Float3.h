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
        struct NumberArray3
        {
            static_assert(std::is_arithmetic_v<T>);

            union
            {
#ifdef IS_MATHS_DIRECTX_MATHS
                struct { DirectX::XMVECTOR xmvector; };
#endif
#if defined(IS_MATHS_GLM) || defined(IS_MATHS_CONSTRUCTOR_GLM) || defined(IS_TESTING)
                struct { glm::vec<3, T, glm::defaultp> vec3; };
#endif
                struct { T x, y, z; };
                struct { T r, g, b; };
                struct { T data[3]; };
            };
        };

        using Float3 = NumberArray3<float>;
        using Int3 = NumberArray3<int>;
        */
    }
}