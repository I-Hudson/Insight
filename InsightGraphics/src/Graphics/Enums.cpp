#include "Graphics/Enums.h"

namespace Insight
{
	namespace Graphics
	{
#define IF_FLAG_BITS_STR(flags, flagBit) if (flags & flagBit) { str += #flagBit; str += " |"; }
#define IF_FLAG_STR(flag, flagBit) if (flag == flagBit) { str += #flagBit; str += " |"; }

		std::string EnumToString(ResourceType type)
		{
			std::string str;

			IF_FLAG_STR(type, ResourceType::Buffer);
			IF_FLAG_STR(type, ResourceType::Texture);

			return str;
		}

		std::string ResourceStateToString(ResourceState state)
		{
			std::string str;

			IF_FLAG_BITS_STR(state, ResourceState::ResourceState_Present);
			IF_FLAG_BITS_STR(state, ResourceState::ResourceState_Render_Target);

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
	}
}