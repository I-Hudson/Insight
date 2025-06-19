#include "Maths/Vector4.h"
#include "Maths/Vector3.h"

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
		const Vec<4, T> Vec<4, T>::One = Vec<4, T>(1.0f);
		template<typename T>
		const Vec<4, T> Vec<4, T>::Zero = Vec<4, T>(0.0f);
		template<typename T>
		const Vec<4, T> Vec<4, T>::Infinity = Vec<4, T>(std::numeric_limits<T>::infinity());
		template<typename T>
		const Vec<4, T> Vec<4, T>::InfinityNeg = Vec<4, T>(-std::numeric_limits<T>::infinity());

		template<typename T>
		Vec<4, T>::Vec()
		{ 
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVectorReplicate(0.0f);
#else
			x = 0.0f; 
			y = 0.0f; 
			z = 0.0f;
			w = 0.0f;
#endif
		}
		template<typename T>
		Vec<4, T>::Vec(T x, T y, T z, T w)
		{ 
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVectorSet(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(w));
#else
			(*this).x = x;
			(*this).y = y;
			(*this).z = z;
			(*this).w = w;
#endif
		}

		template<typename T>
		Vec<4, T>::Vec(T scalar)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVectorSet(static_cast<float>(scalar), static_cast<float>(scalar), static_cast<float>(scalar), static_cast<float>(scalar));
#else
			x = scalar;
			y = scalar;
			z = scalar;
			w = scalar;
#endif
		}

		template<typename T>
		Vec<4, T>::Vec(const Vec<2, T>& other, T z, T w)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = other.xmvector;
			xmvector = DirectX::XMVectorSetZ(xmvector, static_cast<float>(z));
			xmvector = DirectX::XMVectorSetW(xmvector, static_cast<float>(w));
#else
			x = other.x;
			y = other.y;
			(*this).z = z;
			(*this).w = w;
#endif
		}
		template<typename T>
		Vec<4, T>::Vec(Vec<2, T>&& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = other.xmvector;
			xmvector = DirectX::XMVectorSetZ(xmvector, 0.0f);
			xmvector = DirectX::XMVectorSetW(xmvector, 0.0f);
#else
			x = other.x;
			y = other.y;
			z = 0.0f;
			w = 0.0f;
#endif
		}

		template<typename T>
		Vec<4, T>::Vec(const Vec<3, T>& other, T w)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVectorSetW(other.xmvector, static_cast<float>(w));
#else
			x = other.x;
			y = other.y;
			z = other.z;
			this->w = w;
#endif
		}
		template<typename T>
		Vec<4, T>::Vec(Vec<3, T>&& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVectorSetW(other.xmvector, 0.0f);
#else
			x = other.x;
			y = other.y;
			z = other.z;
			w = 0.0;
#endif
		}

		template<typename T>
		Vec<4, T>::Vec(const Vec<4, T>& other)
		{ 
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = other.xmvector;
#else
			x = other.x;
			y = other.y;
			z = other.z;
			w = other.w;
#endif
		}
		template<typename T>
		Vec<4, T>::Vec(Vec<4, T>&& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = other.xmvector;
#else
			x = other.x;
			y = other.y;
			z = other.z;
			w = other.w;
#endif
		}

#ifdef IS_MATHS_DIRECTX_MATHS
		template<typename T>
		Vec<4, T>::Vec(const DirectX::XMVECTOR& other)
		{ 
			xmvector = other;
		}

		template<typename T>
		Vec<4, T>::Vec(DirectX::XMVECTOR&& other)
		{
			xmvector = other;
		}
#endif
#if defined(IS_MATHS_GLM) || defined(IS_MATHS_CONSTRUCTOR_GLM) || defined(IS_TESTING)
		template<typename T>
		Vec<4, T>::Vec(const glm::vec4& other)
		{
			vec4 = other;
		}
		template<typename T>
		Vec<4, T>::Vec(glm::vec4&& other)
		{
			vec4 = other;
		}
#endif

		template<typename T>
		Vec<4, T>::~Vec()
		{ }

		template<typename T>
		T Vec<4, T>::Length() const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVectorGetX(DirectX::XMVector4Length(xmvector));
#elif defined(IS_MATHS_GLM)
			return glm::length(vec4);
#else
			return static_cast<T>(std::sqrt(LengthSquared()));
#endif
		}
		template<typename T>
		T Vec<4, T>::LengthSquared() const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVectorGetX(DirectX::XMVector4LengthSq(xmvector));
#elif defined(IS_MATHS_GLM)
			return glm::length2(vec4);
#else
			return (x * x) + (y * y) + (z * z) + (w * w);
#endif
		}

		template<typename T>
		Vec<4, T>& Vec<4, T>::Normalise()
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVector4Normalize(xmvector);
#elif defined(IS_MATHS_GLM)
			vec4 = glm::normalize(vec4);
#else
			const T length_squared = LengthSquared();
			if (!(length_squared == 1.0f) && length_squared > 0.0f)
			{
				const T length_inverted = 1.0f / Length();
				x *= length_inverted;
				y *= length_inverted;
				z *= length_inverted;
				w *= length_inverted;
			}
#endif
			return *this;
		}
		template<typename T>
		Vec<4, T> Vec<4, T>::Normalised() const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<4, T>(DirectX::XMVector4Normalize(xmvector));
#elif defined(IS_MATHS_GLM)
			return glm::normalize(vec4);
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
		T Vec<4, T>::Dot(const Vec<4, T>& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVectorGetX(DirectX::XMVector4Dot(xmvector, other.xmvector));
#elif defined(IS_MATHS_GLM)
			return glm::dot(vec4, other.vec4);
#else
			return (x * other.x) + (y * other.y) + (z * other.z) + (w * other.w);
#endif
		}

		template<typename T>
		Vec<4, T> Vec<4, T>::Floor() const
		{
			return Maths::Vector4(floorf(x), floorf(y), floorf(z), floorf(w));
		}

		template<typename T>
		T& Vec<4, T>::operator[](int i)
		{
			return data[i];
		}
		template<typename T>
		const T& Vec<4, T>::operator[](int i) const
		{
			return data[i];
		}

		template<typename T>
		bool Vec<4, T>::operator==(const Vec<4, T>& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVector4Equal(xmvector, other.xmvector);
#elif defined(IS_MATHS_GLM)
			return vec4 == other.vec4;
#else
			return Equal(other, std::numeric_limits<T>::epsilon());
#endif
		}
		template<typename T>
		bool Vec<4, T>::operator!=(const Vec<4, T>& other) const
		{
			return !(*this == other);
		}

		template<typename T>
		bool Vec<4, T>::Equal(const Vec<4, T>& other, const T errorRange) const
		{
			const bool xResult = Equals(x, other.x, errorRange);
			const bool yResult = Equals(y, other.y, errorRange);
			const bool zResult = Equals(z, other.z, errorRange);
			const bool wResult = Equals(w, other.w, errorRange);
			return xResult
				&& yResult
				&& zResult
				&& wResult;
		}

		template<typename T>
		bool Vec<4, T>::NotEqual(const Vec<4, T>& other, const T errorRange) const
		{
			return !(Equal(other, errorRange));
		}

		template<typename T>
		Vec<4, T> Vec<4, T>::Reciprocal() const
		{
			return Vec<4, T>(1.0f / x, 1.0f / y, 1.0f / z, 1.0f / w);
		}

		template<typename T>
		Vec<4, T> Vec<4, T>::Cross(const Vec<4, T> vec) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<4, T>(DirectX::XMVector3Cross(xmvector, vec.xmvector));
#elif defined(IS_MATHS_GLM)
			return glm::cross(vec3, vec.vec3);
#else
			const T rX = (y * vec.z) - (z * vec.y);
			const T rY = (z * vec.x) - (x * vec.z);
			const T rZ = (x * vec.y) - (y * vec.x);
			return Vec<4, T>(rX, rY, rZ, 0.0f);
#endif
		}

		template<typename T>
		Vec<4, T> Vec<4, T>::operator-() const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<4, T>(DirectX::XMVectorNegate(xmvector));
#elif defined(IS_MATHS_GLM)
			return -vec4;
#else
			return Vec<4, T>(-x, -y, -z, -w);
#endif
		}

		template<typename T>
		Vec<4, T>& Vec<4, T>::operator=(T scalar)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVectorReplicate(static_cast<float>(scalar));
#elif defined(IS_MATHS_GLM)
			vec4 = glm::vec4(scalar);
#else
			x = scalar;
			y = scalar;
			z = scalar;
			w = scalar;
#endif
			return *this;
		}
		template<typename T>
		Vec<4, T>& Vec<4, T>::operator=(const Vec<4, T>& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = other.xmvector;
#elif defined(IS_MATHS_GLM)
			vec4 = other.vec4;
#else
			x = other.x;
			y = other.y;
			z = other.z;
			w = other.w;
#endif
			return *this;
		}

		template<typename T>
		Vec<4, T> Vec<4, T>::operator*(T scalar) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<4, T>(DirectX::XMVectorMultiply(xmvector, DirectX::XMVectorReplicate(static_cast<float>(scalar))));
#elif defined(IS_MATHS_GLM)
			return Vec<4, T>(vec4 * scalar);
#else
			return Vec<4, T>(x * scalar, y * scalar, z * scalar, w * scalar);
#endif
		}
		template<typename T>
		Vec<4, T> Vec<4, T>::operator*(const Vec<4, T>& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<4, T>(DirectX::XMVectorMultiply(xmvector, other.xmvector));
#elif defined(IS_MATHS_GLM)
			return Vec<4, T>(vec4 * other.vec4);
#else
			return Vec<4, T>(x * other.x, y * other.y, z * other.z, w * other.w);
#endif
		}

		template<typename T>
		Vec<4, T> Vec<4, T>::operator/(T scalar) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<4, T>(DirectX::XMVectorDivide(xmvector, DirectX::XMVectorReplicate(static_cast<float>(scalar))));
#elif defined(IS_MATHS_GLM)
			return Vec<4, T>(vec4 / scalar);
#else
			return Vec<4, T>(x / scalar, y / scalar, z / scalar, w / scalar);
#endif
		}
		template<typename T>
		Vec<4, T> Vec<4, T>::operator/(const Vec<4, T>& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<4, T>(DirectX::XMVectorDivide(xmvector, other.xmvector));
#elif defined(IS_MATHS_GLM)
			return Vec<4, T>(vec4 / other.vec4);
#else
			return Vec<4, T>(x / other.x, y / other.y, z / other.z, w / other.w);
#endif
		}

		template<typename T>
		Vec<4, T> Vec<4, T>::operator+(T scalar) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<4, T>(DirectX::XMVectorAdd(xmvector, DirectX::XMVectorReplicate(static_cast<float>(scalar))));
#elif defined(IS_MATHS_GLM)
			return Vec<4, T>(vec4 + scalar);
#else
			return Vec<4, T>(x + scalar, y + scalar, z + scalar, w + scalar);
#endif
		}
		template<typename T>
		Vec<4, T> Vec<4, T>::operator+(const Vec<4, T>& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<4, T>(DirectX::XMVectorAdd(xmvector, other.xmvector));
#elif defined(IS_MATHS_GLM)
			return Vec<4, T>(vec4 + other.vec4);
#else
			return Vec<4, T>(x + other.x, y + other.y, z + other.z, w + other.w);
#endif
		}

		template<typename T>
		Vec<4, T> Vec<4, T>::operator-(T scalar) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<4, T>(DirectX::XMVectorSubtract(xmvector, DirectX::XMVectorReplicate(static_cast<float>(scalar))));
#elif defined(IS_MATHS_GLM)
			return Vec<4, T>(vec4 - scalar);
#else
			return Vec<4, T>(x - scalar, y - scalar, z - scalar, w - scalar);
#endif
		}
		template<typename T>
		Vec<4, T> Vec<4, T>::operator-(const Vec<4, T>& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<4, T>(DirectX::XMVectorSubtract(xmvector, other.xmvector));
			#elif defined(IS_MATHS_GLM)
			return Vec<4, T>(vec4 - other.vec4);
#else
			return Vec<4, T>(x - other.x, y - other.y, z - other.z, w - other.w);
#endif
		}

		template<typename T>
		Vec<4, T>& Vec<4, T>::operator*=(T scalar)
		{
			*this = Vec<4, T>(*this) * scalar;
			return *this;
		}
		template<typename T>
		Vec<4, T>& Vec<4, T>::operator*=(const Vec<4, T>& other)
		{
			*this = Vec<4, T>(*this) * other;
			return *this;
		}

		template<typename T>
		Vec<4, T>& Vec<4, T>::operator/=(T scalar)
		{
			*this = Vec<4, T>(*this) / scalar;
			return *this;
		}
		template<typename T>
		Vec<4, T>& Vec<4, T>::operator/=(const Vec<4, T>& other)
		{
			*this = Vec<4, T>(*this) / other;
			return *this;
		}

		template<typename T>
		Vec<4, T>& Vec<4, T>::operator+=(T scalar)
		{
			*this = Vec<4, T>(*this) + scalar;
			return *this;
		}
		template<typename T>
		Vec<4, T>& Vec<4, T>::operator+=(const Vec<4, T>& other)
		{
			*this = Vec<4, T>(*this) + other;
			return *this;
		}

		template<typename T>
		Vec<4, T>& Vec<4, T>::operator-=(T scalar)
		{
			*this = Vec<4, T>(*this) - scalar;
			return *this;
		}
		template<typename T>
		Vec<4, T>& Vec<4, T>::operator-=(const Vec<4, T>& other)
		{
			*this = Vec<4, T>(*this) - other;
			return *this;
		}

		template<typename T>
		Vec<3, T> Lerp(const Vec<3, T> vec1, const Vec<3, T> vec2, const float f)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVectorLerp(vec1.xmvector, vec2.xmvector, f);
#else
			const Vec<3, T> scale(f);
			const Vec<3, T> length(vec2 - vec1);
			return (length * scale) + v0;
#endif
		}

		template<typename T>
		Vec<4, T> Lerp(const Vec<4, T> vec1, const Vec<4, T> vec2, const float f)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVectorLerp(vec1.xmvector, vec2.xmvector, f);
#else
			const Vec<4, T> scale(f);
			const Vec<4, T> length(vec2 - vec1);
			return (length * scale) + v0;
#endif
		}

		template<typename T>
		Vec<4, T> Select(const Vec<4, T> vec1, const Vec<4, T> vec2, const Vec<4, T> control)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVectorSelect(vec1.xmvector, vec2.xmvector, control.xmvector);
#else
			Maths::Vector4 Result = {
					(vec1.data[0] & ~control.data[0]) | (vec2.data[0] & control.data[0]),
					(vec1.data[1] & ~control.data[1]) | (vec2.data[1] & control.data[1]),
					(vec1.data[2] & ~control.data[2]) | (vec2.data[2] & control.data[2]),
					(vec1.data[3] & ~control.data[3]) | (vec2.data[3] & control.data[3]),
					};
			return Result.v
#endif
		}

		template<typename T>
		Vec<4, T> Min(const Vec<4, T> vec1, const Vec<4, T> vec2)
		{
			return Maths::Vector4(
				vec1.x < vec2.x ? vec1.x : vec2.x
				, vec1.y < vec2.y ? vec1.y : vec2.y
				, vec1.z < vec2.z ? vec1.z : vec2.z
				, vec1.w < vec2.w ? vec1.w : vec2.w);
		}

		template<typename T>
		Vec<4, T> Max(const Vec<4, T> vec1, const Vec<4, T> vec2)
		{
			return Maths::Vector4(
				vec1.x > vec2.x ? vec1.x : vec2.x
				, vec1.y > vec2.y ? vec1.y : vec2.y
				, vec1.z > vec2.z ? vec1.z : vec2.z
				, vec1.w > vec2.w ? vec1.w : vec2.w);
		}
	}
}