#include "Maths/Vector2.h"
#include "Maths/Matrix2.h"

#include "Maths/MathsUtils.h"

#include <cmath>
#include <limits>

#if defined(IS_MATHS_GLM)
#include <glm/gtx/norm.hpp>
#endif 

namespace Insight
{
	namespace Maths
	{
		template<typename T>
		const Vec<2, T> Vec<2, T>::One = Vec<2, T>(1.0f);
		template<typename T>
		const Vec<2, T> Vec<2, T>::Zero = Vec<2, T>(0.0f);
		template<typename T>
		const Vec<2, T> Vec<2, T>::Infinity = Vec<2, T>(std::numeric_limits<T>::infinity());
		template<typename T>
		const Vec<2, T> Vec<2, T>::InfinityNeg = Vec<2, T>(-std::numeric_limits<T>::infinity());

		template<typename T>
		Vec<2, T>::Vec()
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVectorReplicate(0.0f);
#elif defined(IS_MATHS_GLM)
			vec2 = glm::vec2(0, 0);
#else
			x = 0.0f;
			y = 0.0f;
#endif
		}
		template<typename T>
		Vec<2, T>::Vec(const T x, const T y)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVectorSet(static_cast<float>(x), static_cast<float>(y), 0.0f, 0.0f);
#elif defined(IS_MATHS_GLM)
			vec2 = glm::vec2(x, y);
#else
			this->x = x;
			this->y = y;
#endif
		}
		template<typename T>
		Vec<2, T>::Vec(const T value)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVectorSet(static_cast<float>(value), static_cast<float>(value), 0.0f, 0.0f);
#elif defined(IS_MATHS_GLM)
			vec2 = glm::vec2(value, value);
#else
			x = value;
			y = value;
#endif
		}

		template<typename T>
		Vec<2, T>::Vec(const Vec<2, T>& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = other.xmvector;
#elif defined(IS_MATHS_GLM)
			vec2 = other.vec2;
#else
			x = other.x;
			y = other.y;
#endif
		}
		template<typename T>
		Vec<2, T>::Vec(Vec<2, T>&& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = other.xmvector;
#elif defined(IS_MATHS_GLM)
			vec2 = other.vec2;
#else
			x = other.x;
			y = other.y;
#endif
		}

#ifdef IS_MATHS_DIRECTX_MATHS
		template<typename T>
		Vec<2, T>::Vec(const DirectX::XMVECTOR& other)
		{
			xmvector = other;
		}
		template<typename T>
		Vec<2, T>::Vec(DirectX::XMVECTOR&& other)
		{
			xmvector = other;
		}
#endif
#if defined(IS_MATHS_GLM) || defined(IS_MATHS_CONSTRUCTOR_GLM) || defined(IS_TESTING)
		template<typename T>
		Vec<2, T>::Vec(const glm::vec2& other)
		{
			vec2 = other;
		}
		template<typename T>
		Vec<2, T>::Vec(glm::vec2&& other)
		{
			vec2 = other;
		}
#endif

		template<typename T>
		Vec<2, T>::~Vec()
		{
		}

		template<typename T>
		T Vec<2, T>::Length() const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVectorGetX(DirectX::XMVector2Length(xmvector));
#elif defined(IS_MATHS_GLM)
			return glm::length(vec2);
#else
			return static_cast<T>(sqrt(LengthSquared()));
#endif
		}
		template<typename T>
		T Vec<2, T>::LengthSquared() const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVectorGetX(DirectX::XMVector2LengthSq(xmvector));
#elif defined(IS_MATHS_GLM)
			return glm::length2(vec2);
#else
			return (x * x) + (y * y);
#endif
		}

		template<typename T>
		Vec<2, T>& Vec<2, T>::Normalise()
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVector2Normalize(xmvector);
#elif defined(IS_MATHS_GLM)
			vec2 = glm::normalize(vec2);
#else
			const T length_squared = LengthSquared();
			if (!(length_squared == 1.0f) && length_squared > 0.0f)
			{
				const T length_inverted = 1.0f / Length();
				x *= length_inverted;
				y *= length_inverted;
			}
#endif
			return *this;
		}
		template<typename T>
		Vec<2, T> Vec<2, T>::Normalised() const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<2, T>(DirectX::XMVector2Normalize(xmvector));
#elif defined(IS_MATHS_GLM)
			return glm::normalize(vec2);
#else
			const auto length_squared = LengthSquared();
			if (!(length_squared == 1.0f) && length_squared > 0.0f)
			{
				const T length_inverted = 1.0f / Length();
				return (*this) * length_inverted;
			}
			else
			{
				return *this;
			}
#endif
		}

		template<typename T>
		T Vec<2, T>::Dot(const Vec<2, T>& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVectorGetX(DirectX::XMVector2Dot(xmvector, other.xmvector));
#elif defined(IS_MATHS_GLM)
			return glm::dot(vec2, other.vec2);
#else
			return (x * other.x) + (y * other.y);
#endif
		}

		template<typename T>
		T& Vec<2, T>::operator[](int i)
		{
			return data[i];
		}
		template<typename T>
		const T& Vec<2, T>::operator[](int i) const
		{
			return data[i];
		}

		template<typename T>
		bool Vec<2, T>::operator==(const Vec<2, T>& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVectorGetX(DirectX::XMVectorEqual(xmvector, other.xmvector));
#elif defined(IS_MATHS_GLM)
			return vec2 == other.vec2;
#else
			return Equal(other, std::numeric_limits<T>::epsilon());
#endif
		}
		template<typename T>
		bool Vec<2, T>::operator!=(const Vec<2, T>& other) const
		{
			return !(*this == other);
		}

		template<typename T>
		bool Vec<2, T>::Equal(const Vec<2, T>& other, const T errorRange) const
		{
			return Equals(x, other.x, errorRange) && Equals(y, other.y, errorRange);
		}

		template<typename T>
		bool Vec<2, T>::NotEqual(const Vec<2, T>& other, const T errorRange) const
		{
			return !(Equal(other, errorRange));
		}

		template<typename T>
		Vec<2, T> Vec<2, T>::operator-() const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<2, T>(DirectX::XMVectorNegate(xmvector));
#elif defined(IS_MATHS_GLM)
			return -vec2;
#else
			return Vec<2, T>(-x, -y);
#endif
		}

		template<typename T>
		Vec<2, T>& Vec<2, T>::operator=(T value)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVectorReplicate(static_cast<float>(value));
#elif defined(IS_MATHS_GLM)
			vec2 = glm::vec2(value, value);
#else
			x = value;
			y = value;
#endif
			return *this;
		}
		template<typename T>
		Vec<2, T>& Vec<2, T>::operator=(const Vec<2, T>& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = other.xmvector;
#elif defined(IS_MATHS_GLM)
			vec2 = other.vec2;
#else
			x = other.x;
			y = other.y;
#endif
			return *this;
		}

		template<typename T>
		Vec<2, T> Vec<2, T>::operator*(T scalar) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<2, T>(DirectX::XMVectorMultiply(xmvector, DirectX::XMVectorReplicate(static_cast<float>(scalar))));
#elif defined(IS_MATHS_GLM)
			return Vec<2, T>(vec2 * scalar);
#else
			return Vec<2, T>(x * scalar, y * scalar);
#endif
		}
		template<typename T>
		Vec<2, T> Vec<2, T>::operator*(const Vec<2, T>& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<2, T>(DirectX::XMVectorMultiply(xmvector, other.xmvector));
#elif defined(IS_MATHS_GLM)
			return Vec<2, T>(vec2 * other.vec2);
#else
			return Vec<2, T>(x * other.x, y * other.y);
#endif
		}

		template<typename T>
		Vec<2, T> Vec<2, T>::operator/(T scalar) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<2, T>(DirectX::XMVectorDivide(xmvector, DirectX::XMVectorReplicate(static_cast<float>(scalar))));
#elif defined(IS_MATHS_GLM)
			return Vec<2, T>(vec2 / scalar);
#else
			return Vec<2, T>(x / scalar, y / scalar);
#endif
		}
		template<typename T>
		Vec<2, T> Vec<2, T>::operator/(const Vec<2, T>& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<2, T>(DirectX::XMVectorDivide(xmvector, other.xmvector));
#elif defined(IS_MATHS_GLM)
			return Vec<2, T>(vec2 / other.vec2);
#else
			return Vec<2, T>(x / other.x, y / other.y);
#endif
		}

		template<typename T>
		Vec<2, T> Vec<2, T>::operator+(T scalar) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<2, T>(DirectX::XMVectorAdd(xmvector, DirectX::XMVectorReplicate(static_cast<float>(scalar))));
#elif defined(IS_MATHS_GLM)
			return Vec<2, T>(vec2 + scalar);
#else
			return Vec<2, T>(x + scalar, y + scalar);
#endif
		}
		template<typename T>
		Vec<2, T> Vec<2, T>::operator+(const Vec<2, T>& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<2, T>(DirectX::XMVectorAdd(xmvector, other.xmvector));
#elif defined(IS_MATHS_GLM)
			return Vec<2, T>(vec2 + other.vec2);
#else
			return Vec<2, T>(x + other.x, y + other.y);
#endif
		}

		template<typename T>
		Vec<2, T> Vec<2, T>::operator-(T scalar) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<2, T>(DirectX::XMVectorSubtract(xmvector, DirectX::XMVectorReplicate(static_cast<float>(scalar))));
#elif defined(IS_MATHS_GLM)
			return Vec<2, T>(vec2 - scalar);
#else
			return Vec<2, T>(x - scalar, y - scalar);
#endif
		}
		template<typename T>
		Vec<2, T> Vec<2, T>::operator-(const Vec<2, T>& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<2, T>(DirectX::XMVectorSubtract(xmvector, other.xmvector));
#elif defined(IS_MATHS_GLM)
			return Vec<2, T>(vec2 - other.vec2);
#else
			return Vec<2, T>(x - other.x, y - other.y);
#endif
		}

		template<typename T>
		Vec<2, T>& Vec<2, T>::operator*=(T scalar)
		{
			*this = Vec<2, T>(*this)* scalar;
			return *this;
		}
		template<typename T>
		Vec<2, T>& Vec<2, T>::operator*=(const Vec<2, T>& other)
		{
			*this = Vec<2, T>(*this)* other;
			return *this;
		}

		template<typename T>
		Vec<2, T>& Vec<2, T>::operator/=(T scalar)
		{
			*this = Vec<2, T>(*this) / scalar;
			return *this;
		}
		template<typename T>
		Vec<2, T>& Vec<2, T>::operator/=(const Vec<2, T>& other)
		{
			*this = Vec<2, T>(*this) / other;
			return *this;
		}

		template<typename T>
		Vec<2, T>& Vec<2, T>::operator+=(T scalar)
		{
			*this = Vec<2, T>(*this) + scalar;
			return *this;
		}
		template<typename T>
		Vec<2, T>& Vec<2, T>::operator+=(const Vec<2, T>& other)
		{
			*this = Vec<2, T>(*this) + other;
			return *this;
		}

		template<typename T>
		Vec<2, T>& Vec<2, T>::operator-=(T scalar)
		{
			*this = Vec<2, T>(*this) - scalar;
			return *this;
		}
		template<typename T>
		Vec<2, T>& Vec<2, T>::operator-=(const Vec<2, T>& other)
		{
			*this = Vec<2, T>(*this) - other;
			return *this;
		}
	}
}