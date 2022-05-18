#pragma once

#include "Core/TypeAlias.h"
#include "Core/Defines.h"
#include <string>

namespace Insight
{
	namespace Graphics
	{
		template<typename E, E V> 
		constexpr bool IsValid() 
		{
			// When compiled with clang, `name` will contain a prettified function name,
			// including the enum value name for `V` if valid. For example:
			// "bool IsValid() [E = Fruit, V = Fruit::BANANA]" for valid enum values, or:
			// "bool IsValid() [E = Fruit, V = 10]" for invalid enum values.
			constexpr const char* name = FUNCTION;
			int i = strlen(name);
			// Find the final space character in the pretty name.
			for (; i >= 0; --i)
			{
				if (name[i] == ' ') 
				{
					break;
				}
			}
			// The character after the final space will indicate if
			// it's a valid value or not.
			char c = name[i + 1];
			if (c >= '0' && c <= '9') 
			{
				return false;
			}
			return true;
		}

		template<typename E> 
		constexpr int CountValid() 
		{
			return 0;
		}

		template<typename E, E A, E... B> 
		constexpr int CountValid()
		{
			bool is_valid = IsValid<E, A>();
			return CountValid<E, B...>() + (int)is_valid;
		}

		template<typename E, int... I> 
		constexpr int InternalElementCount(std::integer_sequence<int, I...> unused)
		{
			return CountValid<E, (E)I...>();
		}

		template<typename E> 
		struct ElementCount
		{
			static const int value = InternalElementCount<E>(std::make_integer_sequence<int, 100>());
		};

		template<int Size, typename... Args>
		struct EnumToStringObject
		{ };

		enum class ResourceType
		{
			Buffer,
			Texture,

			Count
		};
		std::string ResourceTypeToString(ResourceType type);

		enum ResourceState
		{
			ResourceState_Present,
			ResourceState_Render_Target
		};
		std::string ResourceStateToString(ResourceState state);

		enum GPUQueue
		{
			GPUQueue_Graphics,
			GPUQueue_Compute,
			GPUQueue_Transfer,
		};
		std::string GPUQueueToString(GPUQueue queue);

		enum class GPUCommandListType
		{
			Default,
			Transient,
			Compute,
			Reset
		};
		std::string GPUCommandListTypeToString(GPUCommandListType type);

		enum ShaderStageFlagBits
		{
			ShaderStage_Vertex			= 1 << 0,
			ShaderStage_TessControl		= 1 << 1,
			ShaderStage_TessEval		= 1 << 2,
			ShaderStage_Geometry		= 1 << 3,
			ShaderStage_Pixel			= 1 << 4,
		};
		const u32 ShaderStageCount = 5;
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
            TransferSrc					= 1 << 0,
            TransferDst					= 1 << 1,
            Sampled						= 1 << 2,
            Storage						= 1 << 3,
            ColourAttachment			= 1 << 4,
            DepthStencilAttachment		= 1 << 5,
            TransientAttachment			= 1 << 6,
            InputAttachment				= 1 << 7
        };
        using ImageUsageFlags = u32;
        std::string ImageUsageFlagsToString(ImageUsageFlags flags);

		enum class DescriptorType
		{
			Sampler,
			Combined_Image_Sampler,
			Sampled_Image,
			Storage_Image,
			Uniform_Texel_Buffer,
			Storage_Texel_Buffer,
			Unifom_Buffer,
			Storage_Buffer,
			Uniform_Buffer_Dynamic,
			Storage_Buffer_Dyanmic,
			Input_Attachment,

			Unknown
		};

		enum class DescriptorResourceType
		{
			Sampler,
			CBV,
			SRV,
			UAV,
			Unknown
		};

		enum class BufferType
		{
			Vertex,
			Index,
			Uniform,
			Storage,
			Raw,
			Staging,
			Readback
		};

		enum class TextureType
		{
			Unknown,

			Tex1D,
			Tex2D,
			Tex3D,
			TexCube,

			Tex2DArray,
			Tex3DArray,
		};

		//enum GUPBufferFlagBits
		//{
		//	None = 1 << 0,
		//	TransferSrc = 1 << 1,
		//	TransferDst = 1 << 2,
		//	UniformTexel = 1 << 3,
		//	StorageTexel = 1 << 4,
		//	Uniform = 1 << 5,
		//	Storage = 1 << 6,
		//	Index = 1 << 7,
		//	Vertex = 1 << 8,
		//	Indirect = 1 << 9,
		//	ShaderDeviceAddress = 1 << 10,
		//};
		//using GUPBufferFlags = u32;
		//std::string GUPBufferFlagsToString(GUPBufferFlags flags);
	
		enum class DeviceExtension : u32
		{
			BindlessDescriptors,
			ExclusiveFullScreen,

			DeviceExtensionCount
		};

	}
}