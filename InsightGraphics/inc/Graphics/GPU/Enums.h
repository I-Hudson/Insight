#pragma once

#include "Core/TypeAlias.h"
#include <string>

namespace Insight
{
	namespace Graphics
	{
		enum GPUQueue
		{
			GPUQueue_Graphics,
			GPUQueue_Compute,
			GPUQueue_Transfer,
		};

		enum ShaderStageFlagBits
		{
			ShaderStage_Vertex,
			ShaderStage_TessControl,
			ShaderStage_TessEval,
			ShaderStage_Geometry,
			ShaderStage_Pixel,

			ShaderStage_Count
		};
		using ShaderStageFlags = u32;
		std::string ShaderStageFlagsToString(ShaderStageFlags flags);


		enum class PrimitiveTopologyType
		{
			PointList = 0,
			LineList = 1,
			LineStrip = 2,
			TriangleList = 3,
			TriangleStrip = 4,
			TriangleFan = 5,
			LineListWithAdjacency = 6,
			LineStripWithAdjacency = 7,
			TriangleListWithAdjacency = 8,
			TriangleStripWithAdjacency = 9,
			PatchList = 10,
		};
		std::string PrimitiveTopologyTypeToString(PrimitiveTopologyType type);

		enum class PolygonMode
		{
			Fill = 0,
			Line = 1,
			Point = 2,
		};
		std::string PolygonModeToString(PolygonMode mode);

		enum class CullMode
		{
			None = 0,
			Front = 1,
			Back = 2,
			FrontAndBack = 3,
		};
		std::string CullModeToString(CullMode mode);

		enum class FrontFace
		{
			CounterClockwise = 0,
			Clockwise = 1,
		};
		std::string FrontFaceToString(FrontFace face);

		enum ColourComponentFlagBits
		{
			ColourComponentR = 1 << 0,
			ColourComponentG = 1 << 1,
			ColourComponentB = 1 << 2,
			ColourComponentA = 1 << 3,
		};
		using ColourComponentFlags = u32;
		std::string ColourComponentFlagsToString(ColourComponentFlags flags);

		enum class BlendFactor
		{
			Zero = 0,
			One = 1,
			SrcColour = 2,
			OneMinusSrcColour = 3,
			DstColour = 4,
			OneMinusDstColour = 5,
			SrcAlpha = 6,
			OneMinusSrcAlpha = 7,
			DstAlpha = 8,
			OneMinusDstAlpha = 9,
			ConstantColour = 10,
			OneMinusConstantColour = 11,
			ConstantAlpha = 12,
			OneMinusConstantAlpha = 13,
			SrcAplhaSaturate = 14,
			Src1Colour = 15,
			OneMinusSrc1Colour = 16,
			Src1Alpha = 17,
			OneMinusSrc1Alpha = 18,
		};
		std::string BlendFactorToString(BlendFactor factor);

		enum class BlendOp
		{
			Add = 0,
			Subtract = 1,
			ReverseSubtract = 2,
			Min = 3,
			Max = 4,
		};
		std::string BlendOpToString(BlendOp op);

        /// <summary>
        /// Define the image usage flags which a input within the 
        /// rendering pipeline can be.
        /// </summary>
        enum ImageUsageFlagsBits
        {
            TransferSrc = 1 << 0,
            TransferDst = 1 << 1,
            Sampled = 1 << 2,
            Storage = 1 << 3,
            ColourAttachment = 1 << 4,
            DepthStencilAttachment = 1 << 5,
            TransientAttachment = 1 << 6,
            InputAttachment = 1 << 7
        };
        using ImageUsageFlags = u32;
        std::string ImageUsageFlagsToString(ImageUsageFlags flags);
	}
}