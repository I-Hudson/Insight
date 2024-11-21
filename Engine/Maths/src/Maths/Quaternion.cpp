#include "Maths/Quaternion.h"
#include "Maths/Vector2.h"
#include "Maths/Utils.h"
#include "Maths/MathsUtils.h"

#include <cmath>
#include <limits>

namespace Insight
{
    namespace Maths
    {
        const Quaternion Quaternion::Identity = Quaternion(1, 0, 0, 0);

        Quaternion::Quaternion(const Maths::Float3& eulerAnglesRad)
        {
            const float f = 0.5f;
            Maths::Vector3 c = Maths::Vector3(std::cos(eulerAnglesRad.x * f), std::cos(eulerAnglesRad.y * f), std::cos(eulerAnglesRad.z * f));
            Maths::Vector3 s = Maths::Vector3(std::sin(eulerAnglesRad.x * f), std::sin(eulerAnglesRad.y * f), std::sin(eulerAnglesRad.z * f));

            this->w = c.x * c.y * c.z + s.x * s.y * s.z;
            this->x = s.x * c.y * c.z - c.x * s.y * s.z;
            this->y = c.x * s.y * c.z + s.x * c.y * s.z;
            this->z = c.x * c.y * s.z - s.x * s.y * c.z;
        }
        Quaternion::Quaternion(const float eulerXRad, const float eulerYRad, const float eulerZRad)
        {
            const float f = 0.5f;
            Maths::Vector3 c = Maths::Vector3(std::cos(eulerXRad * f), std::cos(eulerYRad * f), std::cos(eulerZRad * f));
            Maths::Vector3 s = Maths::Vector3(std::sin(eulerXRad * f), std::sin(eulerYRad * f), std::sin(eulerZRad * f));

            this->w = c.x * c.y * c.z + s.x * s.y * s.z;
            this->x = s.x * c.y * c.z - c.x * s.y * s.z;
            this->y = c.x * s.y * c.z + s.x * c.y * s.z;
            this->z = c.x * c.y * s.z - s.x * s.y * c.z;
        }
        Quaternion::Quaternion(const float w, const float x, const float y, const float z)
            : w(w), x(x), y(y), z(z)
        { }
        Quaternion::~Quaternion()
        { }

        Quaternion Quaternion::FromEulerDegress(const float eulerX, const float eulerY, const float eulerZ)
        {
            return Quaternion(DegreesToRadians(eulerX), DegreesToRadians(eulerY), DegreesToRadians(eulerZ));
        }

        float& Quaternion::operator[](const int index)
        {
            return data[index];
        }

        const float& Quaternion::operator[](const int index) const
        {
            assert(index >= 0 && index < 4);
            return data[index];
        }

        Quaternion& Quaternion::Normalise()
        {
            const float length = Length();
            if (length <= 0.0f)
            {
                *this = Quaternion(1, 0, 0, 0);
                return *this;
            }
            const float oneOver = 1.0f / length;
            *this = Quaternion(w * oneOver, x * oneOver, y * oneOver, z * oneOver);
            return *this;
        }

        float Quaternion::Length() const
        {
            return std::sqrtf(Dot(*this));
        }

        Vector3 Quaternion::ToEuler() const
        {
            Vector3 euler = Vector3::Zero;

            // pitch
            {
                const float rY = 2.0f * (y * z + w * x);
                const float rX = w * w - x * x - y * y + z * z;
                if (Vector2(rX, y).Equal(Vector2::Zero, EPSILON)) //avoid atan2(0,0) - handle singularity - Matiis
                {
                    euler[0] = std::atan2(rX, w);
                }
                else
                {
                    euler[0] = std::atan2(rY, rX);
                }
            }

            // yaw (y-axis rotation)
            {
                float rY = -2.0f * (x * z - w * y);
                rY = Min(Max(rY, -1.0f), 1.0f);
                euler[1] = std::asin(rY);
            }

            // roll
            {
                const float rY = 2.0f * (x * y + w * z);
                const float rX = w * w + x * x - y * y - z * z;
                if (Vector2(rX, rY).Equal(Vector2::Zero, EPSILON)) //avoid atan2(0,0) - handle singularity - Matiis
                {
                    euler[2] = 0.0f;
                }
                else
                {
                    euler[2] = std::atan2(rY, rX);
                }
            }
            return euler;
        }

        Vector3 Quaternion::ToEulerDeg() const
        {
            Vector3 euler = ToEuler();
            return Vector3(RadiansToDegrees(euler.x), RadiansToDegrees(euler.y), RadiansToDegrees(euler.z));
        }

        Quaternion Quaternion::Inversed() const
        {
            return Conjugate(*this) / Dot(*this);
        }
        float Quaternion::Dot(const Quaternion& q) const
        {
            Vector4 tmp(w * q.w, x * q.x, y * q.y, z * q.z);
            return (tmp.x + tmp.y) + (tmp.z + tmp.w);
        }
        Quaternion Quaternion::Slerp(const Quaternion& q, const float time) const
        {
            Quaternion z = q;
            float cosTheta = Dot(q);

            // If cosTheta < 0, the interpolation will take the long way around the sphere.
            // To fix this, one quat must be negated.
            if (cosTheta < 0.0f)
            {
                z = -q;
                cosTheta = -cosTheta;
            }

            // Perform a linear interpolation when cosTheta is close to 1 to avoid side effect of sin(angle) becoming a zero denominator
            if (cosTheta > 1.0f - std::numeric_limits<float>().epsilon())
            {
                // Linear interpolation
                return Quaternion(
                    Lerp(this->w, z.w, time),
                    Lerp(this->x, z.x, time),
                    Lerp(this->y, z.y, time),
                    Lerp(this->z, z.z, time));
            }
            else
            {
                // Essential Mathematics, page 467
                float angle = std::acos(cosTheta);
                return (sin((1.0f - time) * angle) * *this + std::sin(time * angle) * z) / sin(angle);
            }
        }

        Quaternion& Quaternion::operator=(const Quaternion& q)
        {
            w = q.w;
            x = q.x;
            y = q.y;
            z = q.z;
            return *this;
        }


        Quaternion& Quaternion::operator +=(const Quaternion& q)
        {
            w += q.w;
            x += q.x;
            y += q.y;
            z += q.z;
            return *this;
        }
        Quaternion& Quaternion::operator -=(const Quaternion& q)
        {
            w -= q.w;
            x -= q.x;
            y -= q.y;
            z -= q.z;
            return *this;
        }
        Quaternion& Quaternion::operator *=(const Quaternion& q)
        {
            const Quaternion p(*this);

            this->w = p.w * q.w - p.x * q.x - p.y * q.y - p.z * q.z;
            this->x = p.w * q.x + p.x * q.w + p.y * q.z - p.z * q.y;
            this->y = p.w * q.y + p.y * q.w + p.z * q.x - p.x * q.z;
            this->z = p.w * q.z + p.z * q.w + p.x * q.y - p.y * q.x;

            return *this;
        }

        Quaternion& Quaternion::operator *=(const float s)
        {
            *this = Quaternion(w * s, x * s, y * s, z * s);
            return *this;
        }
        Quaternion& Quaternion::operator /=(const float s)
        {
            *this = Quaternion(w / s, x / s, y / s, z / s);
            return *this;
        }

        Quaternion Quaternion::operator+() const
        {
            return *this;
        }

        Quaternion Quaternion::operator-() const
        {
            return Quaternion(-w, -x, -y, -z);
        }


        Quaternion operator+(const Quaternion& q, const Quaternion& p)
        {
            return Quaternion(q) += p;
        }

        Quaternion operator-(const Quaternion& q, const Quaternion& p)
        {
            return Quaternion(q) -= p;
        }

        Quaternion operator*(const Quaternion& q, const Quaternion& p)
        {
            return Quaternion(q) *= p;
        }

        Vector3 operator*(const Quaternion& q, const Vector3& v)
        {
 		    const Vector3 QuatVector(q.x, q.y, q.z);
		    const Vector3 uv(QuatVector.Cross(v));
		    const Vector3 uuv(QuatVector.Cross(uv));

		return v + ((uv * q.w) + uuv) * 2.0f;
        }

        Vector3 operator*(const Vector3& v, const Quaternion& q)
        {
            return q.Inversed() * v;
        }

        Vector4 operator*(const Quaternion& q, const Vector4& v)
        {
            return Maths::Vector4(q * Maths::Vector3(v), v.w);
        }

        Vector4 operator*(const Vector4& v, const Quaternion& q)
        {
            return q.Inversed() * v;
        }

        Quaternion operator*(const Quaternion& q, const float s)
        {
            return Quaternion(q) *= s;
        }

        Quaternion operator*(const float s, const Quaternion& q)
        {
            return q * s;
        }

        Quaternion operator/(const Quaternion& q, const float s)
        {
            return Quaternion(q.w / s, q.x / s, q.y / s, q.z / s);
        }

        bool operator==(const Quaternion& q1, const Quaternion& q2)
        {
            return 
                q1.w == q1.w &&
                q1.x == q1.x &&
                q1.y == q1.y &&
                q1.z == q1.z;
        }

        bool operator!=(const Quaternion& q1, const Quaternion& q2)
        {
            return
                q1.w != q1.w ||
                q1.x != q1.x ||
                q1.y != q1.y ||
                q1.z != q1.z;
        }

        Quaternion Quaternion::Conjugate(const Quaternion& q) const
        {
            return Quaternion(q.w, -q.x, -q.y, -q.z);
        }
    }
}

#ifdef IS_TESTING
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include "doctest.h"
namespace test
{
	using namespace Insight::Maths;
	TEST_SUITE("Quaternion")
	{
		TEST_CASE("Size Of")
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			CHECK(sizeof(Quaternion) == 16);
#else
			CHECK(sizeof(Quaternion) == 16);
#endif
		}

		TEST_CASE("Constructors")
		{
			const float w1 = 0.0f;
			const float x1 = 0.0f;
			const float y1 = 0.0f;
			const float z1 = 0.0f;
			Quaternion q(w1, x1, y1, z1);

			CHECK(q.w == w1);
			CHECK(q.x == x1);
			CHECK(q.y == y1);
			CHECK(q.z == z1);

			const float eulerX = 90.0f;
			const float eulerY = 245.0f;
			const float eulerZ = -35.0f;
			Quaternion eulerQVec(Vector3(eulerX, eulerY, eulerZ));
			Quaternion eulerQ(eulerX, eulerY, eulerZ);
			glm::quat glmEulerQ(glm::vec3(eulerX, eulerY, eulerZ));

			CHECK(eulerQVec.w == glmEulerQ.w);
			CHECK(eulerQVec.x == glmEulerQ.x);
			CHECK(eulerQVec.y == glmEulerQ.y);
			CHECK(eulerQVec.z == glmEulerQ.z);
			CHECK(eulerQVec == eulerQ);
		}

        TEST_CASE("Euler")
        {
            const float w1 = 0.0f;
            const float x1 = 0.0f;
            const float y1 = 0.0f;
            const float z1 = 0.0f;
            Quaternion q(w1, x1, y1, z1);

            CHECK(q.w == w1);
            CHECK(q.x == x1);
            CHECK(q.y == y1);
            CHECK(q.z == z1);

            constexpr float eulerXDeg = 90.0f;
            constexpr float eulerYDeg = 245.0f;
            constexpr float eulerZDeg = -35.0f;

            constexpr float eulerX = DegreesToRadians(eulerXDeg);
            constexpr float eulerY = DegreesToRadians(eulerYDeg);
            constexpr float eulerZ = DegreesToRadians(eulerZDeg);
            Quaternion eulerQVec(Vector3(eulerX, eulerY, eulerZ));
            Quaternion eulerQ(eulerX, eulerY, eulerZ);
            glm::quat glmEulerQ(glm::vec3(eulerX, eulerY, eulerZ));

            CHECK(eulerQVec.w == glmEulerQ.w);
            CHECK(eulerQVec.x == glmEulerQ.x);
            CHECK(eulerQVec.y == glmEulerQ.y);
            CHECK(eulerQVec.z == glmEulerQ.z);
            CHECK(eulerQVec == eulerQ);

            eulerQ = Quaternion::FromEulerDegress(eulerXDeg, eulerYDeg, eulerZDeg);
            glmEulerQ = glm::vec3(eulerX, eulerY, eulerZ);

            Vector3 eulerAngles = eulerQ.ToEuler();
            glm::vec3 glmQuatToEuler = glm::eulerAngles(glmEulerQ);

            CHECK(Equals(eulerAngles.x, glmQuatToEuler.x, 0.0001f));
            CHECK(Equals(eulerAngles.y, glmQuatToEuler.y, 0.0001f));
            CHECK(Equals(eulerAngles.z, glmQuatToEuler.z, 0.0001f));

            glm::vec3 glmEulerDeg = glm::degrees(glmQuatToEuler);

            Vector3 eulerDeg = eulerQ.ToEulerDeg();

            CHECK(Equals(eulerDeg.x, glmEulerDeg.x, 0.0001f));
            CHECK(Equals(eulerDeg.y, glmEulerDeg.y, 0.0001f));
            CHECK(Equals(eulerDeg.z, glmEulerDeg.z, 0.0001f));
        }


		TEST_CASE("Addition")
		{
			const float eulerX = 90.0f;
			const float eulerY = 245.0f;
			const float eulerZ = -35.0f;
			Quaternion eulerQVec(Vector3(eulerX, eulerY, eulerZ));

			const float eulerX2 = 2.0f;
			const float eulerY2 = 84.56f;
			const float eulerZ2 = -175.9f;
			Quaternion eulerQVec2(Vector3(eulerX2, eulerY2, eulerZ2));

			eulerQVec += eulerQVec2;
			Quaternion resultQ = eulerQVec + eulerQVec2;

			glm::quat glmEulerQ(glm::vec3(eulerX, eulerY, eulerZ));
			glm::quat glmEulerQ1(glm::vec3(eulerX2, eulerY2, eulerZ2));

			glmEulerQ += glmEulerQ1;
			glm::quat glmResultQ = glmEulerQ + glmEulerQ1;

			CHECK(eulerQVec == resultQ);

			CHECK(eulerQVec.w == glmEulerQ.w);
			CHECK(eulerQVec.x == glmEulerQ.x);
			CHECK(eulerQVec.y == glmEulerQ.y);
			CHECK(eulerQVec.z == glmEulerQ.z);
		}

		TEST_CASE("Subtraction")
		{
			const float eulerX = 90.0f;
			const float eulerY = 245.0f;
			const float eulerZ = -35.0f;
			Quaternion eulerQVec(Vector3(eulerX, eulerY, eulerZ));

			const float eulerX2 = 2.0f;
			const float eulerY2 = 84.56f;
			const float eulerZ2 = -175.9f;
			Quaternion eulerQVec2(Vector3(eulerX2, eulerY2, eulerZ2));

			eulerQVec -= eulerQVec2;
			Quaternion resultQ = eulerQVec - eulerQVec2;

			glm::quat glmEulerQ(glm::vec3(eulerX, eulerY, eulerZ));
			glm::quat glmEulerQ1(glm::vec3(eulerX2, eulerY2, eulerZ2));

			glmEulerQ -= glmEulerQ1;
			glm::quat glmResultQ = glmEulerQ - glmEulerQ1;

			CHECK(eulerQVec == resultQ);

			CHECK(eulerQVec.w == glmEulerQ.w);
			CHECK(eulerQVec.x == glmEulerQ.x);
			CHECK(eulerQVec.y == glmEulerQ.y);
			CHECK(eulerQVec.z == glmEulerQ.z);
		}

		TEST_CASE("Multiplication")
		{
			const float eulerX = 90.0f;
			const float eulerY = 245.0f;
			const float eulerZ = -35.0f;
			Quaternion eulerQVec(Vector3(eulerX, eulerY, eulerZ));

			const float eulerX2 = 2.0f;
			const float eulerY2 = 84.56f;
			const float eulerZ2 = -175.9f;
			Quaternion eulerQVec2(Vector3(eulerX2, eulerY2, eulerZ2));

			eulerQVec *= eulerQVec2;
			Quaternion resultQ = eulerQVec * eulerQVec2;

			glm::quat glmEulerQ(glm::vec3(eulerX, eulerY, eulerZ));
			glm::quat glmEulerQ1(glm::vec3(eulerX2, eulerY2, eulerZ2));

			glmEulerQ *= glmEulerQ1;
			glm::quat glmResultQ = glmEulerQ * glmEulerQ1;

			CHECK(eulerQVec == resultQ);

			CHECK(eulerQVec.w == glmEulerQ.w);
			CHECK(eulerQVec.x == glmEulerQ.x);
			CHECK(eulerQVec.y == glmEulerQ.y);
			CHECK(eulerQVec.z == glmEulerQ.z);

			Quaternion eulerQVecScaler = eulerQVec;
			eulerQVecScaler *= 4;

			glm::quat glmEulerQScaler = glmEulerQ;
            glmEulerQScaler *= 4;

			CHECK(eulerQVecScaler.w == glmEulerQScaler.w);
			CHECK(eulerQVecScaler.x == glmEulerQScaler.x);
			CHECK(eulerQVecScaler.y == glmEulerQScaler.y);
			CHECK(eulerQVecScaler.z == glmEulerQScaler.z);

            eulerQVec = Quaternion(Vector3(eulerX, eulerY, eulerZ));
            Vector3 vec3 = eulerQVec * Vector3(eulerX2, eulerY2, eulerZ2);

            glmEulerQ = glm::vec3(eulerX, eulerY, eulerZ);
            glm::vec3 glmVec3 = glmEulerQ * glm::vec3(eulerX2, eulerY2, eulerZ2);

            CHECK(eulerQVec.x == glmEulerQ.x);
            CHECK(eulerQVec.y == glmEulerQ.y);
            CHECK(eulerQVec.z == glmEulerQ.z);

            CHECK(vec3.x == glmVec3.x);
            CHECK(vec3.y == glmVec3.y);
            CHECK(vec3.z == glmVec3.z);

            vec3 = Vector3(eulerX2, eulerY2, eulerZ2) * eulerQVec;
            glmVec3 = glm::vec3(eulerX2, eulerY2, eulerZ2) * glmEulerQ;
            CHECK(vec3.x == glmVec3.x);
            CHECK(vec3.y == glmVec3.y);
            CHECK(vec3.z == glmVec3.z);

            eulerQVec = Quaternion(Vector3(eulerX, eulerY, eulerZ));
            Vector4 vec4 = eulerQVec * Vector4(eulerX2, eulerY2, eulerZ2, 45);

            glmEulerQ = glm::vec3(eulerX, eulerY, eulerZ);
            glm::vec4 glmVec4 = glmEulerQ * glm::vec4(eulerX2, eulerY2, eulerZ2, 45);

            CHECK(eulerQVec.x == glmEulerQ.x);
            CHECK(eulerQVec.y == glmEulerQ.y);
            CHECK(eulerQVec.z == glmEulerQ.z);
            CHECK(eulerQVec.w == glmEulerQ.w);

            CHECK(vec4.x == glmVec4.x);
            CHECK(vec4.y == glmVec4.y);
            CHECK(vec4.z == glmVec4.z);
            CHECK(vec4.w == glmVec4.w);

            vec4 = Vector4(eulerX2, eulerY2, eulerZ2, 45) * eulerQVec;
            glmVec4 = glm::vec4(eulerX2, eulerY2, eulerZ2, 45) * glmEulerQ;
            CHECK(vec4.x == glmVec4.x);
            CHECK(vec4.y == glmVec4.y);
            CHECK(vec4.z == glmVec4.z);
            CHECK(vec4.w == glmVec4.w);
		}

		TEST_CASE("Division")
		{
			const float eulerX = 90.0f;
			const float eulerY = 245.0f;
			const float eulerZ = -35.0f;
			Quaternion eulerQVec(Vector3(eulerX, eulerY, eulerZ));
			Quaternion eulerQVecScaler = eulerQVec;
			eulerQVecScaler /= 4;

			glm::quat glmEulerQ(glm::vec3(eulerX, eulerY, eulerZ));
			glm::quat glmEulerQScaler = glmEulerQ;
            glmEulerQScaler /= 4;

			CHECK(eulerQVecScaler.w == glmEulerQScaler.w);
			CHECK(eulerQVecScaler.x == glmEulerQScaler.x);
			CHECK(eulerQVecScaler.y == glmEulerQScaler.y);
			CHECK(eulerQVecScaler.z == glmEulerQScaler.z);

			
		}

        TEST_CASE("Slerp")
        {
            const float eulerX = 90.0f;
            const float eulerY = 245.0f;
            const float eulerZ = -35.0f;

            const float eulerX1 = 4.0f;
            const float eulerY1 = 23.0f;
            const float eulerZ1 = -189.0f;

            Quaternion eulerQVec(Vector3(eulerX, eulerY, eulerZ));
            Quaternion eulerQVec1(Vector3(eulerX1, eulerY1, eulerZ1));

            glm::quat glmEulerQ(glm::vec3(eulerX, eulerY, eulerZ));
            glm::quat glmEulerQ1(glm::vec3(eulerX1, eulerY1, eulerZ1));

            for (size_t i = 0; i < 50; ++i)
            {
                const float t = (1.0f / 50) * i;
                Quaternion slerp = eulerQVec.Slerp(eulerQVec1, t);
                glm::quat glmSlerp = glm::slerp(glmEulerQ, glmEulerQ1, t);


                CHECK(slerp.w == glmSlerp.w);
                CHECK(slerp.x == glmSlerp.x);
                CHECK(slerp.y == glmSlerp.y);
                CHECK(slerp.z == glmSlerp.z);
            }
        }
	}
}
#endif