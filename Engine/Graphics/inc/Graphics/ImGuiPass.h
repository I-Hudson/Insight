#pragma once

#include "Graphics/Defines.h"
#include "Graphics/RenderContext.h"

namespace Insight
{
	namespace Graphics
	{
		class RHI_Buffer;
		class RHI_Texture;

		class IS_GRAPHICS ImGuiPass
		{
			struct ImGuiPassData
			{
				~ImGuiPassData();

				PipelineStateObject Pso;
				ImDrawData ImDrawData;
				std::vector<ImDrawList> CmdList;
				std::vector<ImDrawList*> CmdListPtrs;

				void Reset();
			};

		public:

			void Create();
			void Render();
			void Release();

		private:
			FrameResource<RHI_Buffer*> m_vertex_buffer;
			FrameResource<RHI_Buffer*> m_index_buffer;
			DoubleBufferVector<ImGuiPassData> m_passData;
		};
	}
}