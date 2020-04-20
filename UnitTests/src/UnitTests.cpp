#include <Insight.h>
#include <CppUnitTest.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using namespace Insight::Maths;

TEST_CLASS(UnitTests)
{
#pragma region Vector2
	TEST_METHOD(Vector2Add)
	{
		Vector2 v;
		v.x = v.x + 100.0f;
		v.y = v.y + 500.0f;
		Assert::AreEqual(v.x, 100.0f);
		Assert::AreEqual(v.y, 500.0f);

		v.x += 100.0f;
		v.y += 500.0f;
		Assert::AreEqual(v.x, 200.0f);
		Assert::AreEqual(v.y, 1000.0f);
	}

	TEST_METHOD(Vector2Sub)
	{
		Vector2 v;
		v.x = v.x - 100.0f;
		v.y = v.y - 500.0f;
		Assert::AreEqual(v.x, -100.0f);
		Assert::AreEqual(v.y, -500.0f);

		v.x -= 100.0f;
		v.y -= 500.0f;
		Assert::AreEqual(v.x, -200.0f);
		Assert::AreEqual(v.y, -1000.0f);
	}

	TEST_METHOD(Vector2Mul)
	{
		Vector2 v(1, 1);
		v.x = v.x * 10.0f;
		v.y = v.y * 20.0f;
		Assert::AreEqual(v.x, 10.0f);
		Assert::AreEqual(v.y, 20.0f);

		v.x *= 10.0f;
		v.y *= 20.0f;
		Assert::AreEqual(v.x, 100.0f);
		Assert::AreEqual(v.y, 400.0f);
	}

	TEST_METHOD(Vector2Div)
	{
		Vector2 v(100, 100);
		v.x = v.x / 10.0f;
		v.y = v.y / 20.0f;
		Assert::AreEqual(v.x, 10.0f);
		Assert::AreEqual(v.y, 5.0f);

		v.x /= 10.0f;
		v.y /= 10.0f;
		Assert::AreEqual(v.x, 1.0f);
		Assert::AreEqual(v.y, 0.5f);
	}

	TEST_METHOD(Vector2Dot)
	{
		Vector2 v(1, 1);
		Vector2 v1(-1, -1);
		float dot = v.Dot(v1);
		Assert::AreEqual(dot, -2.0f);
	}

	TEST_METHOD(Vector2Cross)
	{
		Vector2 v(1, -1);
		Vector2 p = v.GetPerpendicular();
		Assert::AreEqual(p.x, -1.0f);
		Assert::AreEqual(p.y, -1.0f);
	}
#pragma endregion

#pragma region Vector3


#pragma endregion
};

class UnitTest : public Insight::Application
{
public:
	UnitTest()
	{

	}

	~UnitTest()
	{

	}
};

Insight::Application* Insight::CreateApplication()
{
	return new UnitTest();
}