#pragma once

#include "Core/TypeAlias.h"
#include "Core/Singleton.h"

#include <string>

namespace Insight
{
	namespace Graphics
	{
#define FORMAT_STAT(Stat, StatDisplayText) \
std::string _CONCAT(Stat, Formated)() { return StatDisplayText + std::to_string(Stat); }

		struct RenderStats : public Core::Singleton<RenderStats>
		{
			u32 DrawCalls = 0;
			u32 DrawIndexedCalls = 0;
			u32 DispatchCalls = 0;

			u32 IndexBufferBindings = 0;
			u32 VertexBufferBindings = 0;

			u32 DescriptorSetBindings = 0;
			u32 PipelineBarriers = 0;

			FORMAT_STAT(DrawCalls, "Draw Calls: ");
			FORMAT_STAT(DrawIndexedCalls, "Draw Indexed Calls: ");
			FORMAT_STAT(DispatchCalls, "Dispatch Calls: ");
			FORMAT_STAT(IndexBufferBindings, "Index Buffer Bindings Calls: ");
			FORMAT_STAT(VertexBufferBindings, "Vertex Buffer Bindings Calls: ");
			FORMAT_STAT(DescriptorSetBindings, "Descriptor Set Bindings Calls: ");
			FORMAT_STAT(PipelineBarriers, "Pipline barriers Calls: ");

			void Draw();
			void Reset();
		};
#undef FORMAT_STAT
	}
}