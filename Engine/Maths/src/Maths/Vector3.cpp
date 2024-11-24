#include "Maths/Vector3.h"

#ifdef IS_TESTING
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include "doctest.h"
namespace test
{
	using namespace Insight::Maths;
	TEST_SUITE("Vector3")
	{
		TEST_CASE("Size Of")
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			CHECK(sizeof(Vector3) == 16);
#else
			CHECK(sizeof(Vector3) == 12);
#endif
		}

		TEST_CASE("Constructors")
		{
			float x1 = 0.0f;
			float y1 = 0.0f;
			float z1 = 0.0f;
			Vector3 vec = Vector3();
			CHECK(vec.x == x1);
			CHECK(vec.y == y1);
			CHECK(vec.z == z1);

			x1 = 24.8f;
			y1 = 64.9f;
			z1 = 256.5f;
			vec = Vector3(x1, y1, z1);
			CHECK(vec.x == x1);
			CHECK(vec.y == y1);
			CHECK(vec.z == z1);

			x1 = 117.117f;
			vec = Vector3(x1);
			CHECK(vec.x == x1);
			CHECK(vec.y == x1);
			CHECK(vec.z == x1);

#ifdef IS_MATHS_DIRECTX_MATHS
			CHECK(x1 == DirectX::XMVectorGetX(vec.xmvector));
			CHECK(x1 == DirectX::XMVectorGetY(vec.xmvector));
			CHECK(x1 == DirectX::XMVectorGetZ(vec.xmvector));
#elif defined(IS_MATHS_GLM)
			CHECK(x1 == vec.vec3.x);
			CHECK(x1 == vec.vec3.y);
			CHECK(x1 == vec.vec3.z);
#endif
		}

		TEST_CASE("Length")
		{
			const float x1 = 747;
			const float y1 = 273;
			const float z1 = 273;
			glm::vec3 glmVec1 = glm::vec3(x1, y1, z1);
			Vector3 one = Vector3(x1, y1, z1);

			CHECK(Equals(one.Length(), glm::length(glmVec1), 0.001f));
			CHECK(Equals(one.LengthSquared(), glm::length2(glmVec1), 0.001f));
		}

		TEST_CASE("Normalise")
		{
			const float x1 = 9809;
			const float y1 = 8156;
			const float z1 = 4834;
			glm::vec3 glmVec1 = glm::vec3(x1, y1, z1);
			Vector3 one = Vector3(x1, y1, z1);

			CHECK(one.Normalised() == Vector3(glm::normalize(glmVec1)));
			one.Normalise();
			CHECK(one == glm::normalize(glmVec1));
		}

		TEST_CASE("Dot")
		{
			const float x1 = 0.2f;
			const float y1 = 0.7f;
			const float z1 = 0.4f;
			
			const float x2 = -0.5f;
			const float y2 = 0.1f;
			const float z2 = -0.15f;

			glm::vec3 glmVec1 = glm::vec3(x1, y1, z1);
			glm::vec3 glmVec2 = glm::vec3(x2, y2, z2);
			Vector3 one = Vector3(x1, y1, z1);
			Vector3 two = Vector3(x2, y2, z2);

			CHECK(Equals(one.Dot(two), glm::dot(glmVec1, glmVec2), 0.001f));

			const float x3 = -1.0f;
			const float y3 = 0.0f;
			const float z3 = 0.0f;
			glmVec2 = glm::vec3(-x3, y3, z3);
			two = Vector3(-x3, y3, z3);
			CHECK(Equals(one.Dot(two), glm::dot(glmVec1, glmVec2), 0.001f));

			const float x4 = -6;
			const float y4 = 8;
			const float z4 = -5;

			const float x5 = 5;
			const float y5 = 12;
			const float z5 = 7;

			glmVec1 = glm::vec3(x4, y4, z4);
			glmVec2 = glm::vec3(x5, y5, z5);
			one = Vector3(x4, y4, z4);
			two = Vector3(x5, y5, z5);
			float result = one.Dot(two);
			CHECK(Equals(result, glm::dot(glmVec1, glmVec2), 0.001f));
		}

		TEST_CASE("Access Operators")
		{
			const float x1 = 10.0f;
			const float y1 = 5.0f;
			const float z1 = -45.0f;

			Vector3 one = Vector3(x1, y1, z1);
			CHECK(Equals(one[0], x1, 0.001f));
			CHECK(Equals(one[1], y1, 0.001f));
			CHECK(Equals(one[2], z1, 0.001f));

			const float x2 = 6589.0f;
			const float y2 = -457234.0f;
			const float z2 = 865473.0f;
			one[0] = x2;
			one[1] = y2;
			one[2] = z2;
			CHECK(Equals(one[0], x2, 0.001f));
			CHECK(Equals(one[1], y2, 0.001f));
			CHECK(Equals(one[2], z2, 0.001f));
		}

		TEST_CASE("Equal")
		{
			const float x1 = 10.0f;
			const float y1 = 5.0f;
			const float z1 = -45.0f;
			Vector3 one = Vector3(x1, y1, z1);
			Vector3 two = Vector3(x1, y1, z1);
			CHECK(one == two);
		}

		TEST_CASE("Not Equal")
		{
			const float x1 = 10.0f;
			const float y1 = 5.0f;
			const float z1 = -45.0f;

			const float x2 = 6589.0f;
			const float y2 = -457234.0f;
			const float z2 = 865473.0f;

			Vector3 one = Vector3(x1, y1, z1);
			Vector3 two = Vector3(x2, y2, z2);
			CHECK(one != two);
		}

		TEST_CASE("Assign")
		{
			const float x1 = 10.0f;
			const float y1 = 5.0f;
			const float z1 = -45.0f;

			Vector3 one = Vector3(x1, y1, z1);
			const float x2 = 20.0f;
			one = x2;
			CHECK(Equals(one.x, x2, 0.001f));
			CHECK(Equals(one.y, x2, 0.001f));
			CHECK(Equals(one.z, x2, 0.001f));

			const float x3 = 55.0f;
			const float y3 = 75.8f;
			const float z3 = -45.0f;
			Vector3 two(x3, y3, z3);
			one = two;
			CHECK(one == two);
		}

		TEST_CASE("Multiplication")
		{
			const float x1 = 6487.0f;
			const float y1 = 2575.0f;
			const float z1 = 845.0f;

			const float x2 = 2006.0f;
			const float y2 = 9618.0f;
			const float z2 = 7453.0f;

			glm::vec3 glmVec1 = glm::vec3(x1, y1, z1);
			glm::vec3 glmVec2 = glm::vec3(x2, y2, z2);
			Vector3 one = Vector3(x1, y1, z1);
			Vector3 two = Vector3(x2, y2, z2);

			Vector3 resultA = one * two;
			Vector3 resultB = two * one;
			CHECK(resultA == glmVec1 * glmVec2);
			CHECK(resultB == glmVec2 * glmVec1);

			const float x3 = 0.15f;
			const float y3 = 75.0f;
			const float z3 = 14.0f;
			glm::vec3 glmVec3 = glmVec1 *= glm::vec3(x3, y3, z3);
			resultA = one *= Vector3(x3, y3, z3);
			CHECK(one == glmVec3);
			CHECK(resultA == one);
		}

		TEST_CASE("Divide")
		{
			const float x1 = 6487.0f;
			const float y1 = 2575.0f;
			const float z1 = 845.0f;

			const float x2 = 2006.0f;
			const float y2 = 9618.0f;
			const float z2 = 7453.0f;

			glm::vec3 glmVec1 = glm::vec3(x1, y1, z1);
			glm::vec3 glmVec2 = glm::vec3(x2, y2, z2);
			Vector3 one = Vector3(x1, y1, z1);
			Vector3 two = Vector3(x2, y2, z2);

			Vector3 resultA = one / two;
			Vector3 resultB = two / one;

			CHECK(resultA == glmVec1 / glmVec2);
			CHECK(resultB == glmVec2 / glmVec1);

			const float x3 = 0.15f;
			const float y3 = 75.0f;
			const float z3 = 14.0f;

			glm::vec3 glmVec3 = glmVec1 /= glm::vec3(x3, y3, z3);
			resultA = one /= Vector3(x3, y3, z3);
			CHECK(resultA == glmVec3);
		}

		TEST_CASE("Addition")
		{
			const float x1 = 6487.0f;
			const float y1 = 2575.0f;
			const float z1 = 845.0f;

			const float x2 = 2006.0f;
			const float y2 = 9618.0f;
			const float z2 = 7453.0f;

			glm::vec3 glmVec1 = glm::vec3(x1, y1, z1);
			glm::vec3 glmVec2 = glm::vec3(x2, y2, z2);
			Vector3 one = Vector3(x1, y1, z1);
			Vector3 two = Vector3(x2, y2, z2);

			Vector3 resultA = one + two;
			Vector3 resultB = two + one;

			CHECK(resultA.Equal(glmVec1 + glmVec2, 0.001f));
			CHECK(resultB.Equal(glmVec2 + glmVec1, 0.001f));

			const float x3 = 0.15f;
			const float y3 = 75.0f;
			const float z3 = 14.0f;
			glm::vec3 glmVec3 = glmVec1 += glm::vec3(x3, y3, z3);
			resultA = one += Vector3(x3, y3, z3);

			CHECK(resultA.Equal(glmVec3, 0.001f));
		}

		TEST_CASE("Minus")
		{
			const float x1 = 6487.0f;
			const float y1 = 2575.0f;
			const float z1 = 845.0f;

			const float x2 = 2006.0f;
			const float y2 = 9618.0f;
			const float z2 = 7453.0f;

			glm::vec3 glmVec1 = glm::vec3(x1, y1, z1);
			glm::vec3 glmVec2 = glm::vec3(x2, y2, z2);
			Vector3 one = Vector3(x1, y1, z1);
			Vector3 two = Vector3(x2, y2, z2);

			Vector3 resultA = one - two;
			Vector3 resultB = two - one;

			CHECK(resultA.Equal(glmVec1 - glmVec2, 0.001f));
			CHECK(resultB.Equal(glmVec2 - glmVec1, 0.001f));


			const float x3 = 952.0f;
			const float y3 = 111.0f;
			const float z3 = -785.0f;
			glm::vec3 glmVec3 = glmVec1 -= glm::vec3(x3, y3, z3);
			resultA = one -= Vector3(x3, y3, z3);

			CHECK(resultA.Equal(glmVec3, 0.001f));
		}

		TEST_CASE("Lerp")
		{
			const float x1 = 6487.0f;
			const float y1 = 2575.0f;
			const float z1 = 845.0f;

			const float x2 = 2006.0f;
			const float y2 = 9618.0f;
			const float z2 = 7453.0f;

			Vector3 one = Vector3(x1, y1, z1);
			Vector3 two = Vector3(x2, y2, z2);

			glm::vec3 glmVec1 = glm::vec3(x1, y1, z1);
			glm::vec3 glmVec2 = glm::vec3(x2, y2, z2);

			for (size_t i = 0; i < 500; ++i)
			{
				const float f = (1.0f / 500) * i;
				Vector3 lerp = one.Lerp(two, f);
				glm::vec3 glmLerp = glm::mix(glmVec1, glmVec2, f);

				CHECK(lerp.x == glmLerp.x);
				CHECK(lerp.y == glmLerp.y);
				CHECK(lerp.z == glmLerp.z);
			}
		}
	}
}
#endif