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
			x = static_cast<T>(0.0f);
			y = static_cast<T>(0.0f);
		}
		template<typename T>
		Vec<2, T>::Vec(const T x, const T y)
		{
			this->x = x;
			this->y = y;
		}
		template<typename T>
		Vec<2, T>::Vec(const T value)
		{
			x = value;
			y = value;
		}

		template<typename T>
		Vec<2, T>::Vec(const Vec<2, T>& other)
		{
			x = other.x;
			y = other.y;
		}
		template<typename T>
		Vec<2, T>::Vec(Vec<2, T>&& other)
		{
			x = other.x;
			y = other.y;
		}

#ifdef IS_MATHS_DIRECTX_MATHS
		template<typename T>
		Vec<2, T>::Vec(const DirectX::XMVECTOR& other)
		{
			x = DirectX::XMVectorGetX(other);
			y = DirectX::XMVectorGetY(other);
		}
		template<typename T>
		Vec<2, T>::Vec(DirectX::XMVECTOR&& other)
		{
			x = DirectX::XMVectorGetX(other);
			y = DirectX::XMVectorGetY(other);
		}
#endif
#if defined(IS_MATHS_GLM) || defined(IS_MATHS_CONSTRUCTOR_GLM) || defined(IS_TESTING)
		template<typename T>
		Vec<2, T>::Vec(const glm::vec2& other)
		{
			x = other.x;
			y = other.y;
		}
		template<typename T>
		Vec<2, T>::Vec(glm::vec2&& other)
		{
			x = other.x;
			y = other.y;
		}
#endif

		template<typename T>
		Vec<2, T>::~Vec()
		{
		}

		template<typename T>
		T Vec<2, T>::Length() const
		{
			return static_cast<T>(sqrt(LengthSquared()));
		}
		template<typename T>
		T Vec<2, T>::LengthSquared() const
		{
			return (x * x) + (y * y);
		}

		template<typename T>
		Vec<2, T>& Vec<2, T>::Normalise()
		{
			*this = Normalised();
		}
		template<typename T>
		Vec<2, T> Vec<2, T>::Normalised() const
		{
			if constexpr (std::is_integral_v<T>)
			{
				// You cannot natively normalize an integer vector cleanly.
				// Return as-is or assert/warn depending on your engine setup.
				return *this;
			}
			else
			{

#if defined(IS_MATHS_GLM)
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
			}
#endif
		}

		template<typename T>
		T Vec<2, T>::Dot(const Vec<2, T>& other) const
		{
			return (x * other.x) + (y * other.y);
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
			return Equal(other, std::numeric_limits<T>::epsilon());
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
			return Vec<2, T>(-x, -y);
		}

		template<typename T>
		Vec<2, T>& Vec<2, T>::operator=(T value)
		{
			x = value;
			y = value;
			return *this;
		}
		template<typename T>
		Vec<2, T>& Vec<2, T>::operator=(const Vec<2, T>& other)
		{
			x = other.x;
			y = other.y;
			return *this;
		}

		template<typename T>
		Vec<2, T> Vec<2, T>::operator*(T scalar) const
		{
			return Vec<2, T>(x * scalar, y * scalar);
		}
		template<typename T>
		Vec<2, T> Vec<2, T>::operator*(const Vec<2, T>& other) const
		{
			return Vec<2, T>(x * other.x, y * other.y);
		}

		template<typename T>
		Vec<2, T> Vec<2, T>::operator/(T scalar) const
		{
			return Vec<2, T>(x / scalar, y / scalar);
		}
		template<typename T>
		Vec<2, T> Vec<2, T>::operator/(const Vec<2, T>& other) const
		{
			return Vec<2, T>(x / other.x, y / other.y);
		}

		template<typename T>
		Vec<2, T> Vec<2, T>::operator+(T scalar) const
		{
			return Vec<2, T>(x + scalar, y + scalar);
		}
		template<typename T>
		Vec<2, T> Vec<2, T>::operator+(const Vec<2, T>& other) const
		{
			return Vec<2, T>(x + other.x, y + other.y);
		}

		template<typename T>
		Vec<2, T> Vec<2, T>::operator-(T scalar) const
		{
			return Vec<2, T>(x - scalar, y - scalar);
		}
		template<typename T>
		Vec<2, T> Vec<2, T>::operator-(const Vec<2, T>& other) const
		{
			return Vec<2, T>(x - other.x, y - other.y);
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