#pragma once

#include "Insight/Core.h"

#include "Vulkan.h"

namespace Insight
{
	namespace Render
	{
		enum ShaderType
		{
			None = 0,
			Vertex,
			Geometry,
			Fragment,
			Compute
		};

		class IS_API ShaderModuleBase
		{
		public:
			ShaderModuleBase();
			~ShaderModuleBase();

		private:

		};
	}
}