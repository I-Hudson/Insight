#include "Maths/Vector2.h"

#ifdef IS_TESTING
#define DOCTEST_CONFIG_IMPLEMENTATION_IN_DLL
#define DOCTEST_CONFIG_IMPLEMENT
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <doctest.h>
namespace test
{
	using namespace Insight::Maths;
	TEST_SUITE("Vector2")
	{
		TEST_CASE("Size Of")
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			CHECK(sizeof(Vector2) == 16);
#else
			CHECK(sizeof(Vector2) == 8);
#endif
		}

		TEST_CASE("Constructors")
		{
			float x1 = 0.0f;
			float y1 = 0.0f;
			Vector2 vec = Vector2();
			CHECK(vec.x == x1);
			CHECK(vec.y == y1);

			x1 = 24.8f;
			y1 = 64.9f;
			vec = Vector2(x1, y1);
			CHECK(vec.x == x1);
			CHECK(vec.y == y1);

			x1 = 117.117f;
			y1 = 117.117f;
			vec = Vector2(x1);
			CHECK(vec.x == x1);
			CHECK(vec.y == x1);

#ifdef IS_MATHS_DIRECTX_MATHS
			CHECK(x1 == DirectX::XMVectorGetX(vec.xmvector));
			CHECK(x1 == DirectX::XMVectorGetY(vec.xmvector));
#elif defined(IS_MATHS_GLM)
			CHECK(x1 == vec.vec2.x);
			CHECK(x1 == vec.vec2.y);
#endif
		}

		TEST_CASE("Length")
		{
			const float v1 = 747;
			const float v2 = 273;
			glm::vec2 glmVec1 = glm::vec2(v1, v2);
			Vector2 one = Vector2(v1, v2);

			CHECK(Equals(one.Length(), glm::length(glmVec1), 0.001f));
			CHECK(Equals(one.LengthSquared(), glm::length2(glmVec1), 0.001f));
		}

		TEST_CASE("Normalise")
		{
			const float v1 = 9809;
			const float v2 = 8156;
			glm::vec2 glmVec1 = glm::vec2(v1, v2);
			Vector2 one = Vector2(v1, v2);

			CHECK(one.Normalised() == Vector2(glm::normalize(glmVec1)));
			one.Normalise();
			CHECK(one == glm::normalize(glmVec1));
		}

		TEST_CASE("Dot")
		{
			const float v1 = 0.2f;
			const float v2 = 0.7f;
			const float v3 = -0.5f;
			const float v4 = 0.1f;

			glm::vec2 glmVec1 = glm::vec2(v1, v2);
			glm::vec2 glmVec2 = glm::vec2(v3, v4);
			Vector2 one = Vector2(v1, v2);
			Vector2 two = Vector2(v3, v4);

			CHECK(Equals(one.Dot(two), glm::dot(glmVec1, glmVec2), 0.001f));

			glmVec2 = glm::vec2(-1.0f, 0.0f);
			two = Vector2(-1.0f, 0.0f);
			CHECK(Equals(one.Dot(two), glm::dot(glmVec1, glmVec2), 0.001f));

			const float v5 = -6;
			const float v6 = 8;
			const float v7 = 5;
			const float v8 = 12;

			glmVec1 = glm::vec2(v5, v6);
			glmVec2 = glm::vec2(v7, v8);
			one = Vector2(v5, v6);
			two = Vector2(v7, v8);
			float result = one.Dot(two);
			CHECK(Equals(result, glm::dot(glmVec1, glmVec2), 0.001f));
		}

		TEST_CASE("Access Operators")
		{
			Vector2 one = Vector2(10.0f, 5.0f);
			CHECK(Equals(one[0], 10.0f, 0.001f));
			CHECK(Equals(one[1], 5.0f, 0.001f));

			one[0] = 50.0f;
			one[1] = 100.0f;
			CHECK(Equals(one[0], 50.0f, 0.001f));
			CHECK(Equals(one[1], 100.0f, 0.001f));
		}

		TEST_CASE("Equal")
		{
			Vector2 one = Vector2(10.0f, 5.0f);
			Vector2 two = Vector2(10.0f, 5.0f);
			CHECK(one == two);
		}

		TEST_CASE("Not Equal")
		{
			Vector2 one = Vector2(10.0f, 5.0f);
			Vector2 two = Vector2(5.0f, 10.0f);
			CHECK(one != two);
		}

		TEST_CASE("Assign")
		{
			Vector2 one = Vector2(10.0f, 5.0f);
			one = 20.0f;
			CHECK(Equals(one.x, 20.0f, 0.001f));
			CHECK(Equals(one.y, 20.0f, 0.001f));

			Vector2 two(55.0f, 75.8f);
			one = two;
			CHECK(one == two);
		}

		TEST_CASE("Multiplication")
		{
			const float x1 = 6487.0f;
			const float y1 = 2575.0f;
			const float x2 = 2006.0f;
			const float y2 = 9618.0f;

			glm::vec2 glmVec1 = glm::vec2(x1, y1);
			glm::vec2 glmVec2 = glm::vec2(x2, y2);
			Vector2 one = Vector2(x1, y1);
			Vector2 two = Vector2(x2, y2);

			Vector2 resultA = one * two;
			Vector2 resultB = two * one;
			CHECK(resultA == glmVec1 * glmVec2);
			CHECK(resultB == glmVec2 * glmVec1);

			const float x3 = 0.15f;
			const float y3 = 75.0f;
			glm::vec2 glmVec3 = glmVec1 *= glm::vec2(x3, y3);
			resultA = one *= Vector2(x3, y3);
			CHECK(one == glmVec3);
			CHECK(resultA == one);
		}

		TEST_CASE("Divide")
		{
			const float x1 = 969.0f;
			const float y1 = 650.0f;
			const float x2 = 485.0f;
			const float y2 = 233.0f;

			glm::vec2 glmVec1 = glm::vec2(x1, y1);
			glm::vec2 glmVec2 = glm::vec2(x2, y2);
			Vector2 one = Vector2(x1, y1);
			Vector2 two = Vector2(x2, y2);

			Vector2 resultA = one / two;
			Vector2 resultB = two / one;

			CHECK(resultA == glmVec1 / glmVec2);
			CHECK(resultB == glmVec2 / glmVec1);

			const float x3 = 952.0f;
			const float y3 = 111.0f;

			glm::vec2 glmVec3 = glmVec1 /= glm::vec2(x3, y3);
			resultA = one /= Vector2(x3, y3);
			CHECK(resultA == glmVec3);
		}

		TEST_CASE("Addition")
		{
			const float x1 = 969.0f;
			const float y1 = 650.0f;
			const float x2 = 485.0f;
			const float y2 = 233.0f;

			glm::vec2 glmVec1 = glm::vec2(x1, y1);
			glm::vec2 glmVec2 = glm::vec2(x2, y2);
			Vector2 one = Vector2(x1, y1);
			Vector2 two = Vector2(x2, y2);

			Vector2 resultA = one + two;
			Vector2 resultB = two + one;

			CHECK(resultA.Equal(glmVec1 + glmVec2, 0.001f));
			CHECK(resultB.Equal(glmVec2 + glmVec1, 0.001f));

			const float x3 = 952.0f;
			const float y3 = 111.0f;
			glm::vec2 glmVec3 = glmVec1 += glm::vec2(x3, y3);
			resultA = one += Vector2(x3, y3);

			CHECK(resultA.Equal(glmVec3, 0.001f));
		}

		TEST_CASE("Minus")
		{
			const float x1 = 522.0f;
			const float y1 = 650.0f;
			const float x2 = 485.0f;
			const float y2 = 744.0f;

			glm::vec2 glmVec1 = glm::vec2(x1, y1);
			glm::vec2 glmVec2 = glm::vec2(x2, y2);
			Vector2 one = Vector2(x1, y1);
			Vector2 two = Vector2(x2, y2);

			Vector2 resultA = one - two;
			Vector2 resultB = two - one;

			CHECK(resultA.Equal(glmVec1 - glmVec2, 0.001f));
			CHECK(resultB.Equal(glmVec2 - glmVec1, 0.001f));


			const float x3 = 952.0f;
			const float y3 = 111.0f;
			glm::vec2 glmVec3 = glmVec1 -= glm::vec2(x3, y3);
			resultA = one -= Vector2(x3, y3);

			CHECK(resultA.Equal(glmVec3, 0.001f));
		}
	}
}
#endif