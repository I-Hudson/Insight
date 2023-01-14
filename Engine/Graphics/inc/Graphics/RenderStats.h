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

#define FORMAT_STAT_VALUE(Stat, Value, StatDisplayText) \
std::string _CONCAT(Stat, Formated)() { return StatDisplayText + std::to_string(Value); }

		struct RenderStats : public Core::Singleton<RenderStats>
		{
			Core::Timer RenderTime;

			static constexpr u8 AverageRenderTimeCount = 128;
			float AverageRenderTime[AverageRenderTimeCount];
			u8 AverageRenderTimeIndex = 0;

			u64 DrawCalls = 0;
			u64 DrawIndexedCalls = 0;
			u64 DispatchCalls = 0;

			u64 IndexBufferBindings = 0;
			u64 VertexBufferBindings = 0;

			u64 DrawIndexedIndicesCount = 0;

			u64 FrameUniformBufferSize = 0;

			u64 DescriptorSetBindings = 0;
			u64 DescriptorSetUpdates = 0;
			u64 DescriptorSetUsedCount = 0;
			u64 PipelineBarriers = 0;

			// DX12 Info
			u64 DescriptorTableResourceCreations = 0;
			u64 DescriptorTableResourceReuse = 0;
			u64 DescriptorTableSamplerCreations = 0;
			u64 DescriptorTableSamplerReuse = 0;

			FORMAT_STAT(DrawCalls, "Draw Calls: ");
			FORMAT_STAT(DrawIndexedCalls, "Draw Indexed Calls: ");
			FORMAT_STAT(DispatchCalls, "Dispatch Calls: ");
			FORMAT_STAT(IndexBufferBindings, "Index Buffer Bindings Calls: ");
			FORMAT_STAT(VertexBufferBindings, "Vertex Buffer Bindings Calls: ");
			FORMAT_STAT(DrawIndexedIndicesCount, "Draw indcies count: ");
			FORMAT_STAT_VALUE(FrameUniformBufferSize, FrameUniformBufferSize / 1024, "Frame Uniform Buffer Size (KB): ");
			FORMAT_STAT(DescriptorSetBindings, "Descriptor Set Bindings Calls: ");
			FORMAT_STAT(DescriptorSetUpdates, "Descriptor Set Update Calls: ");
			FORMAT_STAT(DescriptorSetUsedCount, "Descriptor Set Used Count: ");
			FORMAT_STAT(PipelineBarriers, "Pipline barriers Calls: ");

			FORMAT_STAT(DescriptorTableResourceCreations, "Descriptor Table Resource Creation: ");
			FORMAT_STAT(DescriptorTableResourceReuse, "Descriptor Table Resource Reuse: ");
			FORMAT_STAT(DescriptorTableSamplerCreations, "Descriptor Table Sampler Creation: ");
			FORMAT_STAT(DescriptorTableSamplerReuse, "Descriptor Table Sampler Reuse: ");

			void Draw();
			void Reset();
		};
#undef FORMAT_STAT
#undef FORMAT_STAT_VALUE
	}
}