#pragma once

#include "Core/TypeAlias.h"
#include "Core/Defines.h"
#include "Graphics/PixelFormat.h"

namespace Insight
{
	namespace Graphics
	{
#if 0
		template<typename FlagBits>
		struct Flags
		{
			using MaskType = std::underlying_type_t<FlagBits>;
			using LocalFlag = Flags<FlagBits>;
			static_assert(std::is_same_v<MaskType, int> && "[struct Flags] 'FlagBits' type must be of type 'int'.");
		public:
			CONSTEXPR Flags() NO_EXPECT : m_mask(0) { }
			CONSTEXPR Flags(FlagBits bit) NO_EXPECT : m_mask(static_cast<MaskType>(bit)) { }
			CONSTEXPR Flags(LocalFlag const& rhs) NO_EXPECT : m_mask(rhs.m_mask) { }
			CONSTEXPR Flags(LocalFlag&& rhs) NO_EXPECT : m_mask(rhs.m_mask) { }
			CONSTEXPR explicit Flags(MaskType flags) NO_EXPECT : m_mask(flags) {}

			/// Comparison
			CONSTEXPR bool operator < (LocalFlag const& rhs) const { return m_mask < rhs.m_mask; }
			CONSTEXPR bool operator <=(LocalFlag const& rhs) const { return m_mask <= rhs.m_mask; }
			CONSTEXPR bool operator > (LocalFlag const& rhs) const { return m_mask >= rhs.m_mask; }
			CONSTEXPR bool operator >=(LocalFlag const& rhs) const { return m_mask >= rhs.m_mask; }
			CONSTEXPR bool operator ==(LocalFlag const& rhs) const { return m_mask == rhs.m_mask; }
			CONSTEXPR bool operator !=(LocalFlag const& rhs) const { return m_mask != rhs.m_mask; }

			CONSTEXPR bool operator!() const { return !m_mask; }

			/// Bitwise
			template<typename TOther>
			CONSTEXPR LocalFlag operator&(TOther const& rhs) NO_EXPECT { return LocalFlag(m_mask & static_cast<MaskType>(rhs)); }
			CONSTEXPR LocalFlag operator&(LocalFlag const& rhs) NO_EXPECT { return LocalFlag(m_mask & rhs.m_mask); }
			CONSTEXPR LocalFlag operator|(LocalFlag const& rhs) NO_EXPECT { return LocalFlag(m_mask | rhs.m_mask); }
			CONSTEXPR LocalFlag operator^(LocalFlag const& rhs) NO_EXPECT { return LocalFlag(m_mask ^ rhs.m_mask); }
			///constexpr LocalFlag operator~(LocalFlag const& rhs) NO_EXPECT { return LocalFlag(m_mask ~ rhs.m_mask); }

			/// Assigment 
			CONSTEXPR LocalFlag& operator=(MaskType const& rhs) NO_EXPECT { m_mask = static_cast<MaskType>(rhs); return *this; }
			CONSTEXPR LocalFlag& operator=(LocalFlag const& rhs) NO_EXPECT { m_mask = rhs.m_mask; return *this; }
			CONSTEXPR LocalFlag& operator|=(LocalFlag const& rhs) NO_EXPECT { m_mask |= rhs.m_mask; return *this; }
			CONSTEXPR LocalFlag& operator^=(LocalFlag const& rhs) NO_EXPECT { m_mask ^= rhs.m_mask; return *this; }
			CONSTEXPR LocalFlag& operator&=(LocalFlag const& rhs) NO_EXPECT { m_mask &= rhs.m_mask; return *this; }

			explicit CONSTEXPR operator bool() const NO_EXPECT { return !!m_mask; }
			explicit CONSTEXPR operator MaskType() const NO_EXPECT { return m_mask; }

		private:
			MaskType m_mask  = 0;
		};

		/// @brief HACK. Use '+' to convert a strongly typed enum to it's underlying type (u32).
		/// @tparam T 
		/// @param e 
		/// @return u32
		template <typename T>
		constexpr auto operator+(T e) noexcept
			-> std::enable_if_t<std::is_enum<T>::value, std::underlying_type_t<T>>
		{
			return static_cast<std::underlying_type_t<T>>(e);
		}
#endif 
		using RGTextureHandle = int;

		enum class ResourceType
		{
			Buffer,
			Texture,

			Size
		};
		constexpr const char* ResourceTypeToString[] =
		{
			"Buffer",
			"Texture",
		};
		static_assert(ARRAY_COUNT(ResourceTypeToString) == static_cast<u64>(ResourceType::Size));

		enum GPUQueue
		{
			GPUQueue_Graphics,
			GPUQueue_Compute,
			GPUQueue_Transfer,

			Size
		};
		constexpr const char* GPUQueueToString[] =
		{
			"GPUQueue_Graphics",
			"GPUQueue_Compute",
			"GPUQueue_Transfer",
		};
		static_assert(ARRAY_COUNT(GPUQueueToString) == static_cast<u64>(GPUQueue::Size));

		enum class GPUCommandListType
		{
			Default,
			Transient,
			Compute,
			Reset,

			Size
		};
		constexpr const char* GPUCommandListTypeToString[] =
		{
			"Default",
			"Transient",
			"Compute",
			"Reset",
		};
		static_assert(ARRAY_COUNT(GPUCommandListTypeToString) == static_cast<u64>(GPUCommandListType::Size));

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

			Size
		};
		constexpr const char* PrimitiveTopologyTypeToString[] =
		{
			"PointList",
			"LineList",
			"LineStrip",
			"TriangleList",
			"TriangleStrip",
			"TriangleFan",
			"LineListWithAdjacency",
			"LineStripWithAdjacency",
			"TriangleListWithAdjacency",
			"TriangleStripWithAdjacency",
			"PatchList",
		};
		static_assert(ARRAY_COUNT(PrimitiveTopologyTypeToString) == static_cast<u64>(PrimitiveTopologyType::Size));

		enum class PolygonMode
		{
			Fill = 0,
			Line = 1,
			Point = 2,

			Size
		};
		constexpr const char* PolygonModeToString[] =
		{
			"Fill",
			"Line",
			"Point",
		};
		static_assert(ARRAY_COUNT(PolygonModeToString) == static_cast<u64>(PolygonMode::Size));

		enum class CullMode
		{
			None = 0,
			Front = 1,
			Back = 2,
			FrontAndBack = 3,

			Size
		};
		constexpr const char* CullModeToString[] =
		{
			"None",
			"Front",
			"Back",
			"FrontAndBack",
		};
		static_assert(ARRAY_COUNT(CullModeToString) == static_cast<u64>(CullMode::Size));

		enum class FrontFace
		{
			CounterClockwise = 0,
			Clockwise = 1,

			Size
		};
		constexpr const char* FrontFaceToString[] =
		{
			"CounterClockwise",
			"Clockwise",
		};
		static_assert(ARRAY_COUNT(FrontFaceToString) == static_cast<u64>(FrontFace::Size));

		enum ColourComponentFlagBits
		{
			ColourComponentR = 1 << 0,
			ColourComponentG = 1 << 1,
			ColourComponentB = 1 << 2,
			ColourComponentA = 1 << 3,
		};
		using ColourComponentFlags = u32;

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

			Size
		};
		constexpr const char* BlendFactorToString[] =
		{
			"Zero",
			"One",
			"SrcColour",
			"OneMinusSrcColour",
			"DstColour",
			"OneMinusDstColour",
			"SrcAlpha",
			"OneMinusSrcAlpha",
			"DstAlpha",
			"OneMinusDstAlpha",
			"ConstantColour",
			"OneMinusConstantColour",
			"ConstantAlpha",
			"OneMinusConstantAlpha",
			"SrcAplhaSaturate",
			"Src1Colour",
			"OneMinusSrc1Colour",
			"Src1Alpha",
			"OneMinusSrc1Alpha",
		};
		static_assert(ARRAY_COUNT(BlendFactorToString) == static_cast<u64>(BlendFactor::Size));

		enum class BlendOp
		{
			Add = 0,
			Subtract = 1,
			ReverseSubtract = 2,
			Min = 3,
			Max = 4,

			Size
		};
		constexpr const char* BlendOpToString[] =
		{
			"Add",
			"Subtract",
			"ReverseSubtract",
			"Min",
			"Max",
		};
		static_assert(ARRAY_COUNT(BlendOpToString) == static_cast<u64>(BlendOp::Size));

		enum class CompareOp
		{
			Never,
			Less,
			Equal,
			LessOrEqual,
			Greater,
			NotEqual,
			GreaterOrEqual,
			Always,

			Size
		};
		constexpr const char* CompareOpToString[] =
		{
			"Never",
			"Less",
			"Equal",
			"LessOrEqual",
			"Greater",
			"NotEqual",
			"GreaterOrEqual",
			"Always",
		};
		static_assert(ARRAY_COUNT(CompareOpToString) == static_cast<u64>(CompareOp::Size));

		enum class Filter
		{
			Nearest,
			Linear,
			Point,

			Size
		};
		constexpr const char* FilterToString[] =
		{
			"Nearest",
			"Linear",
			"Point",
		};
		static_assert(ARRAY_COUNT(FilterToString) == static_cast<u64>(Filter::Size));

		enum class SamplerMipmapMode
		{
			Nearest,
			Linear,

			Size
		};
		constexpr const char* SamplerMipmapModeToString[] =
		{
			"Nearest",
			"Linear",
		};
		static_assert(ARRAY_COUNT(SamplerMipmapModeToString) == static_cast<u64>(SamplerMipmapMode::Size));

		enum class SamplerAddressMode
		{
			Repeat,
			MirroredRepeat,
			ClampToEdge,
			ClampToBoarder,
			MirrorClampToEdge,

			Size
		};
		constexpr const char* SamplerAddressModeToString[] =
		{
			"Repeat",
			"MirroredRepeat",
			"ClampToEdge",
			"ClampToBoarder",
			"MirrorClampToEdge",
		};
		static_assert(ARRAY_COUNT(SamplerAddressModeToString) == static_cast<u64>(SamplerAddressMode::Size));

		enum class BorderColour
		{
			FloatTransparentBlack,
			IntTransparentBlack,
			FloatOpaqueBlack,
			IntOpaqueBlack,
			FloatOpaqueWhite,
			IntOpaqueWhite,

			Size
		};
		constexpr const char* BorderColourToString[] =
		{
			"FloatTransparentBlack",
			"IntTransparentBlack",
			"FloatOpaqueBlack",
			"IntOpaqueBlack",
			"FloatOpaqueWhite",
			"IntOpaqueWhite",
		};
		static_assert(ARRAY_COUNT(BorderColourToString) == static_cast<u64>(BorderColour::Size));

		enum class AttachmentLoadOp
		{
			Load,
			Clear,
			DontCare,

			Size
		};
		constexpr const char* AttachmentLoadOpToString[] =
		{
			"Load",
			"Clear",
			"DontCare",
		};
		static_assert(ARRAY_COUNT(AttachmentLoadOpToString) == static_cast<u64>(AttachmentLoadOp::Size));

		enum class AttachmentStoreOp
		{
			Store,
			DontCare,

			Size
		};
		constexpr const char* AttachmentStoreOpToString[] =
		{
			"Store",
			"DontCare"
		};
		static_assert(ARRAY_COUNT(AttachmentStoreOpToString) == static_cast<u64>(AttachmentStoreOp::Size));

        //// <summary>
        //// Define the image usage flags which a input within the 
        //// rendering pipeline can be.
        //// </summary>
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

		enum class PipelineStageFlagBits : u32
		{
			TopOfPipe						= 1 << 0,
			DrawIndirect					= 1 << 1,
			VertexInput						= 1 << 2,
			VertexShader					= 1 << 3,
			TessesllationControlShader		= 1 << 4,
			TessesllationEvaluationShader	= 1 << 5,
			GeometryShader					= 1 << 6,
			FragmentShader					= 1 << 7,
			EarlyFramgmentShader			= 1 << 8,
			LateFramgmentShader				= 1 << 9,
			ColourAttachmentOutput			= 1 << 10,
			ComputeShader					= 1 << 11,
			Transfer						= 1 << 12,
			BottomOfPipe					= 1 << 13,
			Host							= 1 << 14,
			AllGraphics						= 1 << 15,
			AllCommands						= 1 << 16,
		};
		using PipelineStageFlags = u32;

		enum class DescriptorType
		{
			Sampler,
			Sampled_Image,
			Storage_Image,
			Uniform_Texel_Buffer,
			Storage_Texel_Buffer,
			Unifom_Buffer,
			Storage_Buffer,
			Uniform_Buffer_Dynamic,
			Storage_Buffer_Dyanmic,
			Input_Attachment,

			Unknown,
			Size
		};
		constexpr const char* DescriptorTypeToString[] =
		{
			"Sampler",
			"Sampled_Image",
			"Storage_Image",
			"Uniform_Texel_Buffer",
			"Storage_Texel_Buffer",
			"Unifom_Buffer",
			"Storage_Buffer",
			"Uniform_Buffer_Dynamic",
			"Storage_Buffer_Dyanmic",
			"Input_Attachment",

			"Unknown"
		};
		static_assert(ARRAY_COUNT(DescriptorTypeToString) == static_cast<u64>(DescriptorType::Size));

		enum AccessFlagBits
		{
			IndirectCommandRead				= 1 << 0,
			IndexRead						= 1 << 1,
			VertexAttributeRead				= 1 << 2,
			UniformRead						= 1 << 3,
			InputAttachmentRead				= 1 << 4,
			ShaderRead						= 1 << 5,
			ShaderWrite						= 1 << 6,
			ColorAttachmentRead				= 1 << 7,
			ColorAttachmentWrite			= 1 << 8,
			DepthStencilAttachmentRead		= 1 << 9,
			DepthStencilAttachmentWrite		= 1 << 10,
			TransferRead					= 1 << 11,
			TransferWrite					= 1 << 12,
			HostRead						= 1 << 13,
			HostWrite						= 1 << 14,
			MemoryRead						= 1 << 15,
			MemoryWrite						= 1 << 16,
			None							= 1 << 17,
		};
		using AccessFlags = u32;
		PipelineStageFlags AccessFlagBitsToPipelineStageFlag(AccessFlags flags);

		enum class ImageLayout
		{
			Undefined,
			General,
			ColourAttachment,
			DepthStencilAttachment,
			DepthStencilAttachmentReadOnly,
			ShaderReadOnly,
			TransforSrc,
			TransforDst,
			Preinitialised,
			DepthReadOnlyStencilAttacment,
			DepthAttachmentStencilReadOnly,
			DepthAttachmentOnly,
			DepthReadOnly,
			StencilAttacment,
			StencilReadOnly,
			PresentSrc,

			Size
		};
		AccessFlags ImageLayoutToAccessMask(ImageLayout layout);

		constexpr const char* ImageLayoutToString[] =
		{
			"Undefined",
			"General",
			"ColourAttachment",
			"DepthStencilAttachment",
			"DepthStencilAttachmentReadOnly",
			"ShaderReadOnly",
			"TransforSrc",
			"TransforDst",
			"Preinitialised",
			"DepthReadOnlyStencilAttacment",
			"DepthAttachmentStencilReadOnly",
			"DepthAttachmentOnly",
			"DepthReadOnly",
			"StencilAttacment",
			"StencilReadOnly",
			"PresentSrc"
		};
		static_assert(ARRAY_COUNT(ImageLayoutToString) == static_cast<u64>(ImageLayout::Size));

		enum ImageAspectFlagBits
		{
			Colour		= 1 << 0,
			Depth		= 1 << 1,
			Stencil		= 1 << 2,
		};
		using ImageAspectFlags = u32;
		ImageAspectFlags PixelFormatToAspectFlags(PixelFormat format);

		enum class DescriptorResourceType
		{
			Sampler,
			CBV,
			SRV,
			UAV,
			Unknown,

			Size
		};
		constexpr const char* DescriptorResourceTypeToString[] =
		{
			"Sampler",
			"CBV",
			"SRV",
			"UAV",
			"Unknown",
		};
		static_assert(ARRAY_COUNT(DescriptorResourceTypeToString) == static_cast<u64>(DescriptorResourceType::Size));

		enum class BufferType
		{
			Vertex,
			Index,
			Uniform,
			Storage,
			Raw,
			Staging,
			Readback,

			Size
		};
		constexpr const char* BufferTypeToString[] =
		{
			"Vertex",
			"Index",
			"Uniform",
			"Storage",
			"Raw",
			"Staging",
			"Readback",
		};
		static_assert(ARRAY_COUNT(BufferTypeToString) == static_cast<u64>(BufferType::Size));

		enum class TextureType
		{
			Unknown,

			Tex1D,
			Tex2D,
			Tex3D,
			TexCube,

			Tex2DArray,
			Tex3DArray,

			Size,
		};
		constexpr const char* TextureTypeToString[] =
		{
			"Unknown",
			"Tex1D",
			"Tex2D",
			"Tex3D",
			"TexCube",
			"Tex2DArray",
			"Tex3DArray",
		};
		static_assert(ARRAY_COUNT(TextureTypeToString) == static_cast<u64>(TextureType::Size));

		enum class IndexType
		{
			Uint16,
			Uint32,

			Size
		};
		constexpr const char* IndexTypeToString[] =
		{
			"Uint16",
			"Uint32",
		};
		static_assert(ARRAY_COUNT(IndexTypeToString) == static_cast<u64>(IndexType::Size));

		///enum GUPBufferFlagBits
		///{
		///	None = 1 << 0,
		///	TransferSrc = 1 << 1,
		///	TransferDst = 1 << 2,
		///	UniformTexel = 1 << 3,
		///	StorageTexel = 1 << 4,
		///	Uniform = 1 << 5,
		///	Storage = 1 << 6,
		///	Index = 1 << 7,
		///	Vertex = 1 << 8,
		///	Indirect = 1 << 9,
		///	ShaderDeviceAddress = 1 << 10,
		///};
		///using GUPBufferFlags = u32;
		///std::string GUPBufferFlagsToString(GUPBufferFlags flags);
	
		enum class DynamicState
		{
			Viewport, 
			Scissor,
			LineWidth,

			Size
		};
		constexpr const char* DynamicStateToString[] =
		{
			"Viewport",
			"Scissor",
			"LineWidth",
		};
		static_assert(ARRAY_COUNT(DynamicStateToString) == static_cast<u64>(DynamicState::Size));

		enum class DeviceExtension : u32
		{
			BindlessDescriptors,
			ExclusiveFullScreen,

			VulkanDynamicRendering,

			Size
		};
		constexpr const char* DeviceExtensionToString[] =
		{
			"BindlessDescriptors",
			"ExclusiveFullScreen",
			"VulkanDynamicRendering",
		};
		static_assert(ARRAY_COUNT(DeviceExtensionToString) == static_cast<u64>(DeviceExtension::Size));

		enum class RenderOptions
		{
			ReverseZ,

			Size
		};
		constexpr const char* RenderOptionsToString[] =
		{
			"ReverseZ",
		};
		static_assert(ARRAY_COUNT(RenderOptionsToString) == static_cast<u64>(RenderOptions::Size));

		enum class DeviceUploadStatus
		{
			NotUploaded,
			Queued,
			Uploading,
			Completed,

			Size
		};
		constexpr const char* DeviceUploadStatusToString[] =
		{
			"NotUploaded",
			"Queued",
			"Uploading",
			"Completed",
		};
		static_assert(ARRAY_COUNT(DeviceUploadStatusToString) == static_cast<u64>(DeviceUploadStatus::Size));

		/// @brief Define all present modes available to the swap chain.
		enum class SwapchainPresentModes
		{
			Immediate,
			VSync,
			Variable,

			Size
		};
		constexpr const char* SwapchainPresentModesToString[] =
		{
			"Immediate",
			"VSync",
			"Variable",
		};
		static_assert(ARRAY_COUNT(SwapchainPresentModesToString) == static_cast<u64>(SwapchainPresentModes::Size));
	}
}