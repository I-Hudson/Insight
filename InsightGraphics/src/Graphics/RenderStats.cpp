#include "Graphics/RenderStats.h"

#include <imgui.h>

namespace Insight
{
	namespace Graphics
	{
		void RenderStats::Draw()
		{
			ImGui::Begin("Render Stats");

			ImGui::Text(DrawCallsFormated().c_str());
			ImGui::Text(DrawIndexedCallsFormated().c_str());
			ImGui::Text(DispatchCallsFormated().c_str());
			ImGui::Text(IndexBufferBindingsFormated().c_str());
			ImGui::Text(VertexBufferBindingsFormated().c_str());
			ImGui::Text(DescriptorSetBindingsFormated().c_str());
			ImGui::Text(PipelineBarriersFormated().c_str());

			ImGui::End();
			Reset();
		}

		void RenderStats::Reset()
		{
			DrawCalls = 0;
			DrawIndexedCalls = 0;
			DispatchCalls = 0;
			IndexBufferBindings = 0;
			VertexBufferBindings = 0;
			DescriptorSetBindings = 0;
			PipelineBarriers = 0;
		}
	}
}