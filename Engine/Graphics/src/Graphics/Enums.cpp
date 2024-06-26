#include "Graphics/Enums.h"
#include "Graphics/EnumStrings.h"
#include "Graphics/PixelFormatExtensions.h"

#include "Core/Asserts.h"

namespace Insight
{
	namespace Graphics
	{
#define IF_FLAG_BITS_STR(flags, flagBit) if (flags & flagBit) { str += #flagBit; str += " |"; }
#define IF_FLAG_STR(flag, flagBit) if (flag == flagBit) { str += #flagBit; str += " |"; }

#if 0
		std::string EnumToString(ResourceType type)
		{
			std::string str;

			IF_FLAG_STR(type, ResourceType::Buffer);
			IF_FLAG_STR(type, ResourceType::Texture);

			return str;
		}

		std::string GPUQueueToString(GPUQueue queue)
		{
			std::string str;

			IF_FLAG_BITS_STR(queue, GPUQueue::GPUQueue_Graphics);
			IF_FLAG_BITS_STR(queue, GPUQueue::GPUQueue_Compute);
			IF_FLAG_BITS_STR(queue, GPUQueue::GPUQueue_Transfer);

			return str;
		}

		std::string GPUCommandListTypeToString(GPUCommandListType type)
		{
			std::string str;

			IF_FLAG_STR(type, GPUCommandListType::Default);
			IF_FLAG_STR(type, GPUCommandListType::Transient);
			IF_FLAG_STR(type, GPUCommandListType::Compute);
			IF_FLAG_STR(type, GPUCommandListType::Reset);

			return str;
		}
#endif
		std::string ShaderStageFlagsToString(ShaderStageFlags flags)
		{
			std::string str;

			IF_FLAG_BITS_STR(flags, ShaderStageFlagBits::ShaderStage_Vertex);
			IF_FLAG_BITS_STR(flags, ShaderStageFlagBits::ShaderStage_TessControl);
			IF_FLAG_BITS_STR(flags, ShaderStageFlagBits::ShaderStage_TessEval);
			IF_FLAG_BITS_STR(flags, ShaderStageFlagBits::ShaderStage_Geometry);
			IF_FLAG_BITS_STR(flags, ShaderStageFlagBits::ShaderStage_Pixel);

			if (!str.empty())
			{
				str.pop_back();
			}
			return str;
		}
#if 0
		std::string PrimitiveTopologyTypeToString(PrimitiveTopologyType type)
		{
			std::string str;

			IF_FLAG_STR(type, PrimitiveTopologyType::PointList);
			IF_FLAG_STR(type, PrimitiveTopologyType::LineList);
			IF_FLAG_STR(type, PrimitiveTopologyType::LineStrip);
			IF_FLAG_STR(type, PrimitiveTopologyType::TriangleList);
			IF_FLAG_STR(type, PrimitiveTopologyType::TriangleStrip);
			IF_FLAG_STR(type, PrimitiveTopologyType::TriangleFan);
			IF_FLAG_STR(type, PrimitiveTopologyType::LineListWithAdjacency);
			IF_FLAG_STR(type, PrimitiveTopologyType::LineStripWithAdjacency);
			IF_FLAG_STR(type, PrimitiveTopologyType::TriangleListWithAdjacency);
			IF_FLAG_STR(type, PrimitiveTopologyType::TriangleStripWithAdjacency);
			IF_FLAG_STR(type, PrimitiveTopologyType::PatchList);

			if (!str.empty())
			{
				str.pop_back();
			}
			return str;
		}

		std::string PolygonModeToString(PolygonMode mode)
		{
			std::string str;

			IF_FLAG_STR(mode, PolygonMode::Fill);
			IF_FLAG_STR(mode, PolygonMode::Line);
			IF_FLAG_STR(mode, PolygonMode::Point);

			if (!str.empty())
			{
				str.pop_back();
			}
			return str;
		}

		std::string CullModeToString(CullMode mode)
		{
			std::string str;

			IF_FLAG_STR(mode, CullMode::None);
			IF_FLAG_STR(mode, CullMode::Front);
			IF_FLAG_STR(mode, CullMode::Back);
			IF_FLAG_STR(mode, CullMode::FrontAndBack);

			if (!str.empty())
			{
				str.pop_back();
			}
			return str;
		}

		std::string FrontFaceToString(FrontFace face)
		{
			std::string str;

			IF_FLAG_STR(face, FrontFace::CounterClockwise);
			IF_FLAG_STR(face, FrontFace::Clockwise);

			if (!str.empty())
			{
				str.pop_back();
			}
			return str;
		}
#endif
		std::string ColourComponentFlagsToString(ColourComponentFlags flags)
		{
			std::string str;

			IF_FLAG_BITS_STR(flags, ColourComponentFlagBits::ColourComponentR);
			IF_FLAG_BITS_STR(flags, ColourComponentFlagBits::ColourComponentG);
			IF_FLAG_BITS_STR(flags, ColourComponentFlagBits::ColourComponentB);
			IF_FLAG_BITS_STR(flags, ColourComponentFlagBits::ColourComponentA);

			if (!str.empty())
			{
				str.pop_back();
			}
			return str;
		}
#if 0
		std::string BlendFactorToString(BlendFactor factor)
		{
			std::string str;

			IF_FLAG_STR(factor, BlendFactor::Zero);
			IF_FLAG_STR(factor, BlendFactor::One);
			IF_FLAG_STR(factor, BlendFactor::SrcColour);
			IF_FLAG_STR(factor, BlendFactor::OneMinusSrcColour);
			IF_FLAG_STR(factor, BlendFactor::DstColour);
			IF_FLAG_STR(factor, BlendFactor::OneMinusDstColour);
			IF_FLAG_STR(factor, BlendFactor::SrcAlpha);
			IF_FLAG_STR(factor, BlendFactor::OneMinusSrcAlpha);
			IF_FLAG_STR(factor, BlendFactor::DstAlpha);
			IF_FLAG_STR(factor, BlendFactor::OneMinusDstAlpha);
			IF_FLAG_STR(factor, BlendFactor::ConstantColour);
			IF_FLAG_STR(factor, BlendFactor::OneMinusConstantColour);
			IF_FLAG_STR(factor, BlendFactor::ConstantAlpha);
			IF_FLAG_STR(factor, BlendFactor::OneMinusConstantAlpha);
			IF_FLAG_STR(factor, BlendFactor::SrcAplhaSaturate);
			IF_FLAG_STR(factor, BlendFactor::OneMinusSrc1Colour);
			IF_FLAG_STR(factor, BlendFactor::Src1Alpha);
			IF_FLAG_STR(factor, BlendFactor::OneMinusSrc1Alpha);

			if (!str.empty())
			{
				str.pop_back();
			}
			return str;
		}

		std::string BlendOpToString(BlendOp op)
		{
			std::string str;

			IF_FLAG_STR(op, BlendOp::Add);
			IF_FLAG_STR(op, BlendOp::Subtract);
			IF_FLAG_STR(op, BlendOp::ReverseSubtract);
			IF_FLAG_STR(op, BlendOp::Min);
			IF_FLAG_STR(op, BlendOp::Max);

			if (!str.empty())
			{
				str.pop_back();
			}
			return str;
		}

		std::string CompareOpToString(CompareOp op)
		{
			switch (op)
			{
			case Insight::Graphics::CompareOp::Never:			return "Never";
			case Insight::Graphics::CompareOp::Less:			return "Less";
			case Insight::Graphics::CompareOp::Equal:			return "Equal";
			case Insight::Graphics::CompareOp::LessOrEqual:		return "Less Or Equal";
			case Insight::Graphics::CompareOp::Greater:			return "Greater";
			case Insight::Graphics::CompareOp::GreaterOrEqual:	return "Greater Or Equal";
			case Insight::Graphics::CompareOp::Always:			return "Always";
			default:
				break;
			}
			return "";
		}


		std::string FilterToString(Filter filter)
		{
			switch (filter)
			{
			case Filter::Nearest:	return "Nearest";
			case Filter::Linear:	return "Linear";
			case Filter::Point:		return "Point";
			default:
				break;
			}
			return "";
		}

		std::string SamplerMipmapModeToString(SamplerMipmapMode sampler_mipmap_mode)
		{
			switch (sampler_mipmap_mode)
			{
			case SamplerMipmapMode::Nearest:	return "Nearest";
			case SamplerMipmapMode::Linear:		return "Linear";
			default:
				break;
			}
			return "";
		}

		std::string SamplerAddressModeToString(SamplerAddressMode sampler_address_mode)
		{
			switch (sampler_address_mode)
			{
			case SamplerAddressMode::Repeat:			return "Repeat";
			case SamplerAddressMode::MirroredRepeat:	return "Mirrored Repeat";
			case SamplerAddressMode::ClampToEdge:		return "Clamp To Edge";
			case SamplerAddressMode::ClampToBoarder:	return "Clamp To Boarder";
			case SamplerAddressMode::MirrorClampToEdge:	return "Mirror Clamp To Edge";
			default:
				break;
			}
			return "";
		}

		std::string BoarderColourToString(BorderColour boarder_colour)
		{
			switch (boarder_colour)
			{
			case BorderColour::FloatTransparentBlack:	return "Float Transparent Black";
			case BorderColour::IntTransparentBlack:		return "Int Transparent Black";
			case BorderColour::FloatOpaqueBlack:		return "Float Opaque Black";
			case BorderColour::IntOpaqueBlack:			return "Int Opaque Black";
			case BorderColour::FloatOpaqueWhite:		return "Float Opaque White";
			case BorderColour::IntOpaqueWhite:			return "Int Opaque White";
			default:
				break;
			}
			return "";
		}

		std::string AttachmentLoadOpToString(AttachmentLoadOp op)
		{
			std::string str;

			IF_FLAG_STR(op, AttachmentLoadOp::Load);
			IF_FLAG_STR(op, AttachmentLoadOp::Clear);
			IF_FLAG_STR(op, AttachmentLoadOp::DontCare);

			if (!str.empty())
			{
				str.pop_back();
			}
			return str;
		}

		std::string AttacmentStoreOpToString(AttachmentStoreOp op)
		{
			std::string str;

			IF_FLAG_STR(op, AttachmentStoreOp::Store);
			IF_FLAG_STR(op, AttachmentStoreOp::DontCare);

			if (!str.empty())
			{
				str.pop_back();
			}
			return str;
		}
#endif
		AccessFlags ImageLayoutToAccessMask(ImageLayout layout)
		{
			AccessFlags access_flags;
			switch (layout)
			{
			case Insight::Graphics::ImageLayout::Undefined:
				access_flags = AccessFlagBits::None;
				break;

			case Insight::Graphics::ImageLayout::General:
				access_flags = AccessFlagBits::ShaderRead | AccessFlagBits::ShaderWrite;
				break;

			case Insight::Graphics::ImageLayout::ColourAttachment:
				access_flags = AccessFlagBits::ColorAttachmentRead | AccessFlagBits::ColorAttachmentWrite;
				break;

			case Insight::Graphics::ImageLayout::DepthStencilAttachment:
				access_flags = AccessFlagBits::DepthStencilAttachmentRead | AccessFlagBits::DepthStencilAttachmentWrite;
				break;

			case Insight::Graphics::ImageLayout::DepthStencilAttachmentReadOnly:
				access_flags = AccessFlagBits::DepthStencilAttachmentRead;
				break;

			case Insight::Graphics::ImageLayout::ShaderReadOnly:
				access_flags = AccessFlagBits::ShaderRead;
				break;

			case Insight::Graphics::ImageLayout::ShaderWrite:
				access_flags = AccessFlagBits::ShaderWrite;
				break;

			case Insight::Graphics::ImageLayout::TransforSrc:
				access_flags |= AccessFlagBits::TransferRead;
				break;

			case Insight::Graphics::ImageLayout::TransforDst:
				access_flags |= AccessFlagBits::TransferWrite;
				break;

			case Insight::Graphics::ImageLayout::Preinitialised:
				break;

			case Insight::Graphics::ImageLayout::DepthReadOnlyStencilAttacment:
				access_flags = AccessFlagBits::DepthStencilAttachmentRead;
				break;

			case Insight::Graphics::ImageLayout::DepthAttachmentStencilReadOnly:
				access_flags = AccessFlagBits::DepthStencilAttachmentRead;
				break;

			case Insight::Graphics::ImageLayout::DepthAttachmentOnly:
				access_flags = AccessFlagBits::DepthStencilAttachmentRead;
				break;

			case Insight::Graphics::ImageLayout::DepthReadOnly:
				access_flags = AccessFlagBits::ShaderRead;
				break;

			case Insight::Graphics::ImageLayout::StencilAttacment:
				FAIL_ASSERT();
				break;

			case Insight::Graphics::ImageLayout::StencilReadOnly:
				access_flags |= AccessFlagBits::MemoryRead;
				break;

			case Insight::Graphics::ImageLayout::PresentSrc:
				access_flags |= AccessFlagBits::MemoryRead;
				break;

			default:
				FAIL_ASSERT();
				break;
			}

			return access_flags;
		}

		std::string ImageUsageFlagsToString(ImageUsageFlags flags)
		{
			std::string str;

			IF_FLAG_BITS_STR(flags, ImageUsageFlagsBits::TransferSrc);
			IF_FLAG_BITS_STR(flags, ImageUsageFlagsBits::TransferDst);
			IF_FLAG_BITS_STR(flags, ImageUsageFlagsBits::Sampled);
			IF_FLAG_BITS_STR(flags, ImageUsageFlagsBits::Storage);
			IF_FLAG_BITS_STR(flags, ImageUsageFlagsBits::ColourAttachment);
			IF_FLAG_BITS_STR(flags, ImageUsageFlagsBits::DepthStencilAttachment);
			IF_FLAG_BITS_STR(flags, ImageUsageFlagsBits::TransientAttachment);
			IF_FLAG_BITS_STR(flags, ImageUsageFlagsBits::InputAttachment);

			if (!str.empty())
			{
				str.pop_back();
			}
			return str;
		}

		std::string AccessFlagBitsToString(AccessFlagBits flags)
		{
			std::string str;

			IF_FLAG_BITS_STR(flags, AccessFlagBits::IndirectCommandRead);
			IF_FLAG_BITS_STR(flags, AccessFlagBits::IndexRead);
			IF_FLAG_BITS_STR(flags, AccessFlagBits::VertexAttributeRead);
			IF_FLAG_BITS_STR(flags, AccessFlagBits::UniformRead);
			IF_FLAG_BITS_STR(flags, AccessFlagBits::InputAttachmentRead);
			IF_FLAG_BITS_STR(flags, AccessFlagBits::ShaderRead);
			IF_FLAG_BITS_STR(flags, AccessFlagBits::ShaderWrite);
			IF_FLAG_BITS_STR(flags, AccessFlagBits::ColorAttachmentRead);
			IF_FLAG_BITS_STR(flags, AccessFlagBits::ColorAttachmentWrite);
			IF_FLAG_BITS_STR(flags, AccessFlagBits::DepthStencilAttachmentRead);
			IF_FLAG_BITS_STR(flags, AccessFlagBits::DepthStencilAttachmentWrite);
			IF_FLAG_BITS_STR(flags, AccessFlagBits::TransferRead);
			IF_FLAG_BITS_STR(flags, AccessFlagBits::TransferWrite);
			IF_FLAG_BITS_STR(flags, AccessFlagBits::HostRead);
			IF_FLAG_BITS_STR(flags, AccessFlagBits::HostWrite);
			IF_FLAG_BITS_STR(flags, AccessFlagBits::MemoryRead);
			IF_FLAG_BITS_STR(flags, AccessFlagBits::MemoryWrite);

			if (!str.empty())
			{
				str.pop_back();
			}
			return str;
		}

		std::string ImageAspectFlagsToString(ImageAspectFlags flags)
		{
			std::string str;

			IF_FLAG_BITS_STR(flags, ImageAspectFlagBits::Colour);
			IF_FLAG_BITS_STR(flags, ImageAspectFlagBits::Depth);
			IF_FLAG_BITS_STR(flags, ImageAspectFlagBits::Stencil);

			if (!str.empty())
			{
				str.pop_back();
			}
			return str;
		}

		PipelineStageFlags AccessFlagBitsToPipelineStageFlag(AccessFlags flags)
		{
			PipelineStageFlags pipeline_flags = 0;
			u32 access_flags = flags;
			while (access_flags != 0)
			{
				AccessFlagBits access_flag_bit = static_cast<AccessFlagBits>(access_flags & (~(access_flags - 1)));
				ASSERT(access_flag_bit != 0 && (access_flag_bit & (access_flag_bit - 1)) == 0);
				access_flags &= ~access_flag_bit;

				switch (access_flag_bit)
				{
				case AccessFlagBits::IndirectCommandRead:
					pipeline_flags |= static_cast<u32>(PipelineStageFlagBits::DrawIndirect);
					break;

				case AccessFlagBits::IndexRead:
					pipeline_flags |= static_cast<u32>(PipelineStageFlagBits::VertexInput);
					break;

				case AccessFlagBits::VertexAttributeRead:
					pipeline_flags |= static_cast<u32>(PipelineStageFlagBits::VertexInput);
					break;

				case AccessFlagBits::UniformRead:
					pipeline_flags |= static_cast<u32>(PipelineStageFlagBits::ComputeShader);
					break;

				case AccessFlagBits::InputAttachmentRead:
					pipeline_flags |= static_cast<u32>(PipelineStageFlagBits::FragmentShader);
					break;

					// Shader
				case AccessFlagBits::ShaderRead:
					pipeline_flags |= static_cast<u32>(PipelineStageFlagBits::ComputeShader);
					break;

				case AccessFlagBits::ShaderWrite:
					pipeline_flags |= static_cast<u32>(PipelineStageFlagBits::ComputeShader);
					break;

					// Color attachments
				case AccessFlagBits::ColorAttachmentRead:
					pipeline_flags |= static_cast<u32>(PipelineStageFlagBits::ColourAttachmentOutput);
					break;

				case AccessFlagBits::ColorAttachmentWrite:
					pipeline_flags |= static_cast<u32>(PipelineStageFlagBits::ColourAttachmentOutput);
					break;

					// Depth stencil attachments
				case AccessFlagBits::DepthStencilAttachmentRead:
					pipeline_flags |= static_cast<u32>(PipelineStageFlagBits::FragmentShader);
					break;

				case AccessFlagBits::DepthStencilAttachmentWrite:
					pipeline_flags |= static_cast<u32>(PipelineStageFlagBits::LateFramgmentShader);
					break;

					// Transfer
				case AccessFlagBits::TransferRead:
					pipeline_flags |= static_cast<u32>(PipelineStageFlagBits::Transfer);
					break;

				case AccessFlagBits::TransferWrite:
					pipeline_flags |= static_cast<u32>(PipelineStageFlagBits::Transfer);
					break;

					// Host
				case AccessFlagBits::HostRead:
					pipeline_flags |= static_cast<u32>(PipelineStageFlagBits::Host);
					break;

				case AccessFlagBits::HostWrite:
					pipeline_flags |= static_cast<u32>(PipelineStageFlagBits::Host);
					break;
				}
			}
			return pipeline_flags;
		}

		ImageAspectFlags PixelFormatToAspectFlags(PixelFormat format)
		{
			ImageAspectFlags aspect_flags;

			if (!PixelFormatExtensions::IsDepthStencil(format))
			{
				aspect_flags |= ImageAspectFlagBits::Colour;
			}
			else
			{
				if (PixelFormatExtensions::IsDepth(format))
				{
					aspect_flags |= ImageAspectFlagBits::Depth;
				}

				if (PixelFormatExtensions::IsDepthStencil(format))
				{
					aspect_flags = ImageAspectFlagBits::Depth | ImageAspectFlagBits::Stencil;
				}
			}

			return aspect_flags;
		}

#if 0
		std::string DynamicStateToString(DynamicState dynamic_state)
		{
			switch (dynamic_state)
			{
			case Insight::Graphics::DynamicState::Viewport:		return "Viewport";
			case Insight::Graphics::DynamicState::Scissor:		return "Scissor";
			case Insight::Graphics::DynamicState::LineWidth:	return "LineWidth";
			default:
				break;
			}
			return "";
		}
#endif
	}
}