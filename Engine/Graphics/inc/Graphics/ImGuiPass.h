#pragma once

#include "Graphics/Defines.h"
#include "Graphics/RenderGraph/RenderGraph.h"

namespace Insight
{
	namespace Graphics
	{
		class RHI_Buffer;
		class RHI_Texture;

		class IS_GRAPHICS ImGuiPass
		{
		public:

			void Create();
			void Render();
			void Release();

		private:
			FrameResource<RHI_Buffer*> m_vertex_buffer;
			FrameResource<RHI_Buffer*> m_index_buffer;
		};
	}
}