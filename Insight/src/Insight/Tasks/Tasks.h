#pragma once

#include "Insight/Core.h"
#include "Insight/Templates/TSingleton.h"
#include <taskflow.hpp>

namespace Insight
{
	namespace Tasks
	{
		class Tasks : public TSingleton<Tasks>
		{
		public:
			Tasks();
			~Tasks();

			template<typename T>
			static void CreateTask();

		private:

		};

		template<typename T>
		inline void Tasks::CreateTask()
		{

		}
	}
}