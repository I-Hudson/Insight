#pragma once

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
			RHI_Buffer* m_vertex_buffer = nullptr;
			RHI_Buffer* m_index_buffer = nullptr;
			RHI_Texture* m_font_texture = nullptr;
		};
	}
}