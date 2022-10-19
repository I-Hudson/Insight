#pragma once

#include "Core/TypeAlias.h"
#include "Core/Singleton.h"
#include "Core/Timer.h"

#include <string>

namespace Insight
{
	namespace Graphics
	{
#define FORMAT_STAT(Stat, StatDisplayText) \
std::string _CONCAT(Stat, Formated)() { return StatDisplayText + std::to_string(Stat); }

		struct RenderStats : public Core::Singleton<RenderStats>
		{
			Core::Timer RenderTime;

			static constexpr u8 AverageRenderTimeCount = 128;
			float AverageRenderTime[AverageRenderTimeCount];
			u8 AverageRenderTimeIndex = 0;

			u32 DrawCalls = 0;
			u32 DrawIndexedCalls = 0;
			u32 DispatchCalls = 0;

			u32 IndexBufferBindings = 0;
			u32 VertexBufferBindings = 0;

			u64 DrawIndexedIndicesCount = 0;

			u32 DescriptorSetBindings = 0;
			u32 DescriptorSetUpdates = 0;
			u32 DescriptorSetUsedCount = 0;
			u32 PipelineBarriers = 0;

			FORMAT_STAT(DrawCalls, "Draw Calls: ");
			FORMAT_STAT(DrawIndexedCalls, "Draw Indexed Calls: ");
			FORMAT_STAT(DispatchCalls, "Dispatch Calls: ");
			FORMAT_STAT(IndexBufferBindings, "Index Buffer Bindings Calls: ");
			FORMAT_STAT(VertexBufferBindings, "Vertex Buffer Bindings Calls: ");
			FORMAT_STAT(DrawIndexedIndicesCount, "Draw indcies count: ");
			FORMAT_STAT(DescriptorSetBindings, "Descriptor Set Bindings Calls: ");
			FORMAT_STAT(DescriptorSetUpdates, "Descriptor Set Update Calls: ");
			FORMAT_STAT(DescriptorSetUsedCount, "Descriptor Set Used Count: ");
			FORMAT_STAT(PipelineBarriers, "Pipline barriers Calls: ");

			void Draw();
			void Reset();
		};
#undef FORMAT_STAT
	}
}