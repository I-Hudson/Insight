#pragma once

#include "Graphics/Defines.h"

#include "Core/TypeAlias.h"
#include "Core/Singleton.h"
#include "Core/Timer.h"

#include "Core/Collections/DoubleBufferVector.h"

#include <string>
#include <sstream>

#include <locale>
#include <iostream>
#include <iomanip>

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

IS_GRAPHICS std::string FormatU64ToCommaString(const Insight::DoubleBufferVector<u64>& value);

namespace Insight
{
	namespace Graphics
	{
#define FORMAT_STAT(Stat, StatDisplayText) \
std::string _CONCAT(Stat, Formated)() { return StatDisplayText + std::to_string(Stat.GetCurrent()); }

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

			DoubleBufferVector<u64> MeshCount;

			DoubleBufferVector<u64> DrawCalls;
			DoubleBufferVector<u64> DrawIndexedCalls;
			DoubleBufferVector<u64> DispatchCalls;

			DoubleBufferVector<u64> IndexBufferBindings;
			DoubleBufferVector<u64> VertexBufferBindings;

			DoubleBufferVector<u64> DrawIndexedIndicesCount;

			DoubleBufferVector<u64> FrameUniformBufferSize;

			DoubleBufferVector<u64> DescriptorSetBindings;
			DoubleBufferVector<u64> DescriptorSetUpdates;
			DoubleBufferVector<u64> DescriptorSetUsedCount;
			DoubleBufferVector<u64> PipelineBarriers;

			// DX12 Info
			DoubleBufferVector<u64> DescriptorTableResourceCreations;
			DoubleBufferVector<u64> DescriptorTableResourceReuse;
			DoubleBufferVector<u64> DescriptorTableSamplerCreations;
			DoubleBufferVector<u64> DescriptorTableSamplerReuse;

			FORMAT_STAT(MeshCount, "Mesh Count: ");
			FORMAT_STAT(DrawCalls, "Draw Calls: ");
			FORMAT_STAT(DrawIndexedCalls, "Draw Indexed Calls: ");
			FORMAT_STAT(DispatchCalls, "Dispatch Calls: ");
			FORMAT_STAT(IndexBufferBindings, "Index Buffer Bindings Calls: ");
			FORMAT_STAT(VertexBufferBindings, "Vertex Buffer Bindings Calls: ");
			FORMAT_STAT_FUNC(DrawIndexedIndicesCount, FormatU64ToCommaString(DrawIndexedIndicesCount), "Draw indcies count: ");
			FORMAT_STAT_VALUE(FrameUniformBufferSize, FrameUniformBufferSize.GetCurrent() / 1024, "Frame Uniform Buffer Size (KB): ");
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