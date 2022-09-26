#pragma once

namespace Insight
{
	namespace Core
	{
		class IManager
		{
		public:

			virtual void OnCreate() = 0;
			virtual void OnUpdate(const float deltaTime);
			virtual void OnDestory() = 0;

		private:
		};

		class SystemManager
		{

		};
	}
}