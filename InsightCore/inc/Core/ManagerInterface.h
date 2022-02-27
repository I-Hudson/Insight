#pragma once

namespace Insight
{
	namespace Core
	{
		class ManagerInterface
		{
		public:

			virtual bool Init() = 0;
			virtual void Update(const float deltaTime) = 0;
			virtual void Destroy() = 0;
		};
	}
}