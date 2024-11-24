#include "Maths/Vector3.h"
#include "Maths/MathsUtils.h"
#include <limits>

#if defined(IS_MATHS_GLM)
#include <glm/gtx/norm.hpp>
#endif 

namespace Insight
{
	namespace Maths
	{
		template<typename T>
		const Vec<3, T> Vec<3, T>::One			= Vec<3, T>(1.0f);
		template<typename T>
		const Vec<3, T> Vec<3, T>::Zero			= Vec<3, T>(0.0f);
		template<typename T>
		const Vec<3, T> Vec<3, T>::Infinity		= Vec<3, T>(std::numeric_limits<T>::infinity());
		template<typename T>
		const Vec<3, T> Vec<3, T>::InfinityNeg	= Vec<3, T>(-std::numeric_limits<T>::infinity());

		template<typename T>
		Vec<3, T>::Vec()
		{ 
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVectorReplicate(0.0f);
#elif defined(IS_MATHS_GLM)
			vec3 = glm::vec3(0, 0, 0);
#else
			x = 0.0f;
			y = 0.0f;
			z = 0.0f;
#endif
		}

		template<typename T>
		Vec<3, T>::Vec(T x, T y, T z)
		{ 
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVectorSet(x, y, z, 0.0f);
#elif defined(IS_MATHS_GLM)
			vec3 = glm::vec3(x, y, z);
#else
			this->x = x;
			this->y = y;
			this->z = z;
#endif
		}

		template<typename T>
		Vec<3, T>::Vec(T scalar)
		{ 
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVectorSet(scalar, scalar, scalar, 0.0f);
#elif defined(IS_MATHS_GLM)
			vec3 = glm::vec3(scalar, scalar, scalar);
#else
			x = scalar;
			y = scalar;
			z = scalar;
#endif
		}

		template<typename T>
		Vec<3, T>::Vec(const Vec<2, T>& other, T z)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = other.xmvector;
			xmvector = DirectX::XMVectorSetZ(xmvector, z);
#else
			x = other.x;
			y = other.y;
			z = z;
#endif
		}
		template<typename T>
		Vec<3, T>::Vec(Vec<2, T>&& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = other.xmvector;
			xmvector = DirectX::XMVectorSetZ(xmvector, z);
#else
			x = other.x;
			y = other.y;
			z = 0.0f;
#endif
		}

		template<typename T>
		Vec<3, T>::Vec(const Vec<3, T>& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = other.xmvector;
#else
			x = other.x;
			y = other.y;
			z = other.z;
#endif
		}
		template<typename T>
		Vec<3, T>::Vec(Vec<3, T>&& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = other.xmvector;
#else
			x = other.x;
			y = other.y;
			z = other.z;
#endif
		}

		template<typename T>
		Vec<3, T>::Vec(const Vec<4, T>& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = other.xmvector;
			xmvector = DirectX::XMVectorSetW(xmvector, 0.0f);
#elif defined(IS_MATHS_GLM)
			vec3 = glm::vec3(other.vec4.x, other.vec4.y, other.vec4.z);
#else
			x = other.x;
			y = other.y; 
			z = other.z;
#endif
		}

		template<typename T>
		Vec<3, T>::Vec(Vec<4, T>&& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = other.xmvector;
			xmvector = DirectX::XMVectorSetW(xmvector, 0.0f);
#elif defined(IS_MATHS_GLM)
			vec3 = glm::vec3(other.vec4.x, other.vec4.y, other.vec4.z);
#else
			x = other.x; 
			y = other.y; 
			z = other.z;
#endif
		}

#ifdef IS_MATHS_DIRECTX_MATHS
		template<typename T>
		Vec<3, T>::Vec(const DirectX::XMVECTOR& other)
		{ 
			xmvector = other;
		}
		template<typename T>
		Vec<3, T>::Vec(DirectX::XMVECTOR&& other)
		{
			xmvector = other;
		}
#endif
#if defined(IS_MATHS_GLM) || defined(IS_MATHS_CONSTRUCTOR_GLM) || defined(IS_TESTING)
		template<typename T>
		Vec<3, T>::Vec(const glm::vec3& other)
		{ 
			vec3 = other;
		}
		template<typename T>
		Vec<3, T>::Vec(glm::vec3&& other)
		{ 
			vec3 = other;
		}
#endif

		template<typename T>
		Vec<3, T>::~Vec()
		{ }

		template<typename T>
		T Vec<3, T>::Length() const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVectorGetX(DirectX::XMVector3Length(xmvector));
#elif defined(IS_MATHS_GLM)
			return glm::length(vec3);
#else
			return static_cast<T>(sqrt(LengthSquared()));
#endif
		}
		template<typename T>
		T Vec<3, T>::LengthSquared() const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(xmvector));
#elif defined(IS_MATHS_GLM)
			return glm::length2(vec3);
#else
			return (x * x) + (y * y) + (z * z);
#endif
		}

		template<typename T>
		Vec<3, T>& Vec<3, T>::Normalise()
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			 xmvector = DirectX::XMVector3Normalize(xmvector);
#elif defined(IS_MATHS_GLM)
			vec3 = glm::normalize(vec3);
#else
			const T length_squared = LengthSquared();
			if (!(length_squared == 1.0f) && length_squared > 0.0f)
			{
				const T length_inverted = 1.0f / Length();
				x *= length_inverted;
				y *= length_inverted;
				z *= length_inverted;
			}
#endif
			return *this;
		}
		template<typename T>
		Vec<3, T> Vec<3, T>::Normalised() const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<3, T>(DirectX::XMVector3Normalize(xmvector));
#elif defined(IS_MATHS_GLM)
			return glm::normalize(vec3);
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
		T Vec<3, T>::Dot(const Vec<3, T>& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVectorGetX(DirectX::XMVector3Dot(xmvector, other.xmvector));
#elif defined(IS_MATHS_GLM)
			return glm::dot(vec3, other.vec3);
#else
			return (x * other.x) + (y * other.y) + (z * other.z);
#endif
		}
		template<typename T>
		Vec<3, T> Vec<3, T>::Cross(const Vec<3, T>& other) const
		{
			return Vec<3, T>(
				y * other.z - other.y * z,
				z * other.x - other.z * x,
				x * other.y - other.x * y);
		}

		template<typename T>
		T& Vec<3, T>::operator[](int i)
		{
			return data[i];
		}
		template<typename T>
		const T& Vec<3, T>::operator[](int i) const
		{
			return data[i];
		}

		template<typename T>
		bool Vec<3, T>::operator==(const Vec<3, T>& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVectorGetX(DirectX::XMVectorEqual(xmvector, other.xmvector));
#elif defined(IS_MATHS_GLM)
			return vec3 == other.vec3;
#else
			return Equal(other, std::numeric_limits<T>::epsilon());
#endif
		}
		template<typename T>
		bool Vec<3, T>::operator!=(const Vec<3, T>& other) const
		{
			return !(*this == other);
		}

		template<typename T>
		bool Vec<3, T>::Equal(const Vec<3, T>& other, const T errorRange) const
		{
			return Equals(x, other.x, errorRange) && Equals(y, other.y, errorRange) && Equals(z, other.z, errorRange);
		}

		template<typename T>
		bool Vec<3, T>::NotEqual(const Vec<3, T>& other, const T errorRange) const
		{
			return !(Equal(other, errorRange));
		}

		template<typename T>
		Vec<3, T> Vec<3, T>::Lerp(const Vec<3, T>& vec, const T time) const
		{
			return Vec<3, T>(*this * (1 - time) + (vec * time));
		}

		template<typename T>
		Vec<3, T> Vec<3, T>::operator-() const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<3, T>(DirectX::XMVectorNegate(xmvector));
#elif defined(IS_MATHS_GLM)
			return -vec3;
#else
			return Vec<3, T>(-x, -y, -z);
#endif
		}

		template<typename T>
		Vec<3, T>& Vec<3, T>::operator=(T scalar)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVectorSet(scalar, scalar, scalar, 0.0f);
#elif defined(IS_MATHS_GLM)
			vec3 = glm::vec3(scalar);
#else
			x = scalar;
			y = scalar;
			z = scalar;
#endif
			return *this;
		}
		template<typename T>
		Vec<3, T>& Vec<3, T>::operator=(const Vec<3, T>& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = other.xmvector;
#elif defined(IS_MATHS_GLM)
			vec3 = other.vec3;
#else
			x = other.x;
			y = other.y;
			z = other.z;
#endif
			return *this;
		}

		template<typename T>
		Vec<3, T> Vec<3, T>::operator*(T scalar) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<3, T>(DirectX::XMVectorMultiply(xmvector, DirectX::XMVectorSet(scalar, scalar, scalar, 0.0f)));
#elif defined(IS_MATHS_GLM)
			return Vec<3, T>(vec3 * scalar);
#else
			return Vec<3, T>(x * scalar, y * scalar, z * scalar);
#endif
		}
		template<typename T>
		Vec<3, T> Vec<3, T>::operator*(const Vec<3, T>& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<3, T>(DirectX::XMVectorMultiply(xmvector, other.xmvector));
#elif defined(IS_MATHS_GLM)
			return Vec<3, T>(vec3 * other.vec3);
#else
			return Vec<3, T>(x * other.x, y * other.y, z * other.z);
#endif
		}

		template<typename T>
		Vec<3, T> Vec<3, T>::operator/(T scalar) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<3, T>(DirectX::XMVectorDivide(xmvector, DirectX::XMVectorSet(scalar, scalar, scalar, 0.0f)));
#elif defined(IS_MATHS_GLM)
			return Vec<3, T>(vec3 / scalar);
#else
			return Vec<3, T>(x / scalar, y / scalar, z / scalar);
#endif
		}
		template<typename T>
		Vec<3, T> Vec<3, T>::operator/(const Vec<3, T>& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<3, T>(DirectX::XMVectorDivide(xmvector, other.xmvector));
#elif defined(IS_MATHS_GLM)
			return Vec<3, T>(vec3 / other.vec3);
#else
			return Vec<3, T>(x / other.x, y / other.y, z / other.z);
#endif
		}

		template<typename T>
		Vec<3, T> Vec<3, T>::operator+(T scalar) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<3, T>(DirectX::XMVectorAdd(xmvector, DirectX::XMVectorSet(scalar, scalar, scalar, 0.0f)));
#elif defined(IS_MATHS_GLM)
			return Vec<3, T>(vec3 + scalar);
#else
			return Vec<3, T>(x + scalar, y + scalar, z + scalar);
#endif
		}
		template<typename T>
		Vec<3, T> Vec<3, T>::operator+(const Vec<3, T>& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<3, T>(DirectX::XMVectorAdd(xmvector, other.xmvector));
#elif defined(IS_MATHS_GLM)
			return Vec<3, T>(vec3 + other.vec3);
#else
			return Vec<3, T>(x + other.x, y + other.y, z + other.z);
#endif
		}

		/*
		Vec<3, T> Vec<3, T>::operator-(T scalar) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<3, T>(DirectX::XMVectorSubtract(xmvector, DirectX::XMVectorSet(scalar, scalar, scalar, 0.0f)));
#elif defined(IS_MATHS_GLM)
			return Vec<3, T>(vec3 - scalar);
#else
			return Vec<3, T>(x - scalar, y - scalar, z - scalar);
#endif
		}
		Vec<3, T> Vec<3, T>::operator-(const Vec<3, T>& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<3, T>(DirectX::XMVectorSubtract(xmvector, other.xmvector));
#elif defined(IS_MATHS_GLM)
			return Vec<3, T>(vec3 - other.vec3);
#else
			return Vec<3, T>(x - other.x, y - other.y, z - other.z);
#endif
		}
		*/

		template<typename T>
		Vec<3, T> operator-(T scalar, const Vec<3, T>& vec)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<3, T>(DirectX::XMVectorSubtract(DirectX::XMVectorSet(scalar, scalar, scalar, 0.0f), vec.xmvector));
#elif defined(IS_MATHS_GLM)
			return Vec<3, T>(scalar - vec);
#else
			return Vec<3, T>(scalar - vec.x, scalar - vec.y, scalar - vec.z);
#endif
		}
		template<typename T>
		Vec<3, T> operator-(const Vec<3, T>& vec, T scalar)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<3, T>(DirectX::XMVectorSubtract(vec.xmvector, DirectX::XMVectorSet(scalar, scalar, scalar, 0.0f)));
#elif defined(IS_MATHS_GLM)
			return Vec<3, T>(vec - scalar);
#else
			return Vec<3, T>(vec.x - scalar, vec.y - scalar, vec.z - scalar);
#endif
		}
		template<typename T>
		Vec<3, T> operator-(const Vec<3, T>& vec, const Vec<3, T>& vec1)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<3, T>(DirectX::XMVectorSubtract(vec.xmvector, vec1.xmvector));
#elif defined(IS_MATHS_GLM)
			return Vec<3, T>(vec.vec3 - vec1.vec3);
#else
			return Vec<3, T>(vec.x - vec1.x, vec.y - vec1.y, vec.z - vec1.z);
#endif
		}

		template<typename T>
		Vec<3, T>& Vec<3, T>::operator*=(T scalar)
		{
			*this = Vec<3, T>(*this) * scalar;
			return *this;
		}
		template<typename T>
		Vec<3, T>& Vec<3, T>::operator*=(const Vec<3, T>& other)
		{
			*this = Vec<3, T>(*this) * other;
			return *this;
		}

		template<typename T>
		Vec<3, T>& Vec<3, T>::operator/=(T scalar)
		{
			*this = Vec<3, T>(*this) / scalar;
			return *this;
		}
		template<typename T>
		Vec<3, T>& Vec<3, T>::operator/=(const Vec<3, T>& other)
		{
			*this = Vec<3, T>(*this) / other;
			return *this;
		}

		template<typename T>
		Vec<3, T>& Vec<3, T>::operator+=(T scalar)
		{
			*this = Vec<3, T>(*this) + scalar;
			return *this;
		}
		template<typename T>
		Vec<3, T>& Vec<3, T>::operator+=(const Vec<3, T>& other)
		{
			*this = Vec<3, T>(*this) + other;
			return *this;
		}

		template<typename T>
		Vec<3, T>& Vec<3, T>::operator-=(T scalar)
		{
			*this = Vec<3, T>(*this) - scalar;
			return *this;
		}
		template<typename T>
		Vec<3, T>& Vec<3, T>::operator-=(const Vec<3, T>& other)
		{
			*this = Vec<3, T>(*this) - other;
			return *this;
		}

		template<typename T>
		T Vector3Distance(const Vec<3, T>& a, const Vec<3, T>& b)
		{
			return (a - b).Length();
		}
	}
}