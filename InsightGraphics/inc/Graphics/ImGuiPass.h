#pragma once

#include "Graphics/RenderGraph/RenderGraph.h"

namespace Insight
{
	namespace Graphics
	{
		class RHI_Buffer;
		class RHI_Texture;

		class ImGuiPass
		{
		public:

			void Render();
			void Release();

		private:
			FrameResource<RHI_Buffer*> m_vertex_buffer;
			FrameResource<RHI_Buffer*> m_index_buffer;
			RHI_Texture* m_font_texture = nullptr;
		};
	}
}