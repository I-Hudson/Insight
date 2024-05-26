#pragma once

#include "Graphics/Defines.h"

#include "Core/TypeAlias.h"
#include "Core/Singleton.h"
#include "Core/Timer.h"

#include <string>

#include <locale>

class comma_numpunct : public std::numpunct<char>
{
protected:
	virtual char do_thousands_sep() const
	{
		return ',';
	}

	virtual std::string do_grouping() const
	{
		return "\03";
	}
};

IS_GRAPHICS std::string FormatU64ToCommaString(u64 value);

namespace Insight
{
	namespace Graphics
	{
#define FORMAT_STAT(Stat, StatDisplayText) \
std::string _CONCAT(Stat, Formated)() { return StatDisplayText + std::to_string(Stat); }

#define FORMAT_STAT_VALUE(Stat, Value, StatDisplayText) \
std::string _CONCAT(Stat, Formated)() { return StatDisplayText + std::to_string(Value); }

#define FORMAT_STAT_FUNC(Stat, Func, StatDisplayText) \
std::string _CONCAT(Stat, Formated)() { return StatDisplayText + Func; }

		struct IS_GRAPHICS RenderStats : public Core::Singleton<RenderStats>
		{
			Core::Timer RenderTime;

			static constexpr u8 AverageRenderTimeCount = 128;
			float AverageRenderTime[AverageRenderTimeCount];
			u8 AverageRenderTimeIndex;

			u64 MeshCount;

			u64 DrawCalls;
			u64 DrawIndexedCalls;
			u64 DispatchCalls;

			u64 IndexBufferBindings;
			u64 VertexBufferBindings;

			u64 DrawIndexedIndicesCount;

			u64 FrameUniformBufferSize;

			u64 DescriptorSetBindings;
			u64 DescriptorSetUpdates;
			u64 DescriptorSetUsedCount;
			u64 PipelineBarriers;

			// DX12 Info
			u64 DescriptorTableResourceCreations;
			u64 DescriptorTableResourceReuse;
			u64 DescriptorTableSamplerCreations;
			u64 DescriptorTableSamplerReuse;

			FORMAT_STAT(MeshCount, "Mesh Count: ");
			FORMAT_STAT(DrawCalls, "Draw Calls: ");
			FORMAT_STAT(DrawIndexedCalls, "Draw Indexed Calls: ");
			FORMAT_STAT(DispatchCalls, "Dispatch Calls: ");
			FORMAT_STAT(IndexBufferBindings, "Index Buffer Bindings Calls: ");
			FORMAT_STAT(VertexBufferBindings, "Vertex Buffer Bindings Calls: ");
			FORMAT_STAT_FUNC(DrawIndexedIndicesCount, FormatU64ToCommaString(DrawIndexedIndicesCount), "Draw indcies count: ");
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