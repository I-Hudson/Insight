#include "Graphics/GPU/Enums.h"

namespace Insight
{
	namespace Graphics
	{
        std::string ShaderStageFlagsToString(ShaderStageFlags flags)
        {
            std::string str;
            if (flags & ShaderStageFlagBits::ShaderStage_Vertex) { str += "ShaderStage_Vertex |"; }
            if (flags & ShaderStageFlagBits::ShaderStage_TessControl) { str += "ShaderStage_TessControl |"; }
            if (flags & ShaderStageFlagBits::ShaderStage_TessEval) { str += "ShaderStage_TessEval |"; }
            if (flags & ShaderStageFlagBits::ShaderStage_Geometry) { str += "ShaderStage_Geometry |"; }
            if (flags & ShaderStageFlagBits::ShaderStage_Pixel) { str += "ShaderStage_Pixel |"; }
            if (!str.empty())
            {
                str.pop_back();
            }
            return str;
        }

        std::string PrimitiveTopologyTypeToString(PrimitiveTopologyType type)
        {
            switch (type)
            {
            case PrimitiveTopologyType::PointList: return "PointList";
            case PrimitiveTopologyType::LineList: return "LineList";
            case PrimitiveTopologyType::LineStrip: return "LineStrip";
            case PrimitiveTopologyType::TriangleList: return "TriangleList";
            case PrimitiveTopologyType::TriangleStrip:return "TriangleStrip";
            case PrimitiveTopologyType::TriangleFan: return "TriangleFan";
            case PrimitiveTopologyType::LineListWithAdjacency: return "LineListWithAdjacency";
            case PrimitiveTopologyType::LineStripWithAdjacency: return "LineStripWithAdjacency";
            case PrimitiveTopologyType::TriangleListWithAdjacency: return "TriangleListWithAdjacency";
            case PrimitiveTopologyType::TriangleStripWithAdjacency: return "TriangleStripWithAdjacency";
            case PrimitiveTopologyType::PatchList: return "PatchList";
            default:
                break;
            }
            return std::string();
        }

        std::string PolygonModeToString(PolygonMode mode)
        {
            switch (mode)
            {
            case PolygonMode::Fill: return "Fill";
            case PolygonMode::Line: return "Line";
            case PolygonMode::Point: return "Point";
            default:
                break;
            }
            return std::string();
        }

        std::string CullModeToString(CullMode mode)
        {
            switch (mode)
            {
            case CullMode::None: return "None";
            case CullMode::Front: return "Front";
            case CullMode::Back: return "Back";
            case CullMode::FrontAndBack: return "FrontAndBack";
            default:
                break;
            }
            return std::string();
        }

        std::string FrontFaceToString(FrontFace face)
        {
            switch (face)
            {
            case FrontFace::CounterClockwise: return "CounterClockwise";
            case FrontFace::Clockwise: return "Clockwise";
            default:
                break;
            }
            return std::string();
        }

        std::string ColourComponentFlagsToString(ColourComponentFlags flags)
        {
            std::string str;
            if (flags & ColourComponentR) { str += "ColourComponentR |"; }
            if (flags & ColourComponentG) { str += "ColourComponentG |"; }
            if (flags & ColourComponentB) { str += "ColourComponentB |"; }
            if (flags & ColourComponentA) { str += "ColourComponentA |"; }
            if (!str.empty())
            {
                str.pop_back();
            }
            return str;
        }

        std::string BlendFactorToString(BlendFactor factor)
        {
            switch (factor)
            {
            case BlendFactor::Zero: return "Zero";
            case BlendFactor::One: return "One";
            case BlendFactor::SrcColour: return "SrcColour";
            case BlendFactor::OneMinusSrcColour: return "OneMinusSrcColour";
            case BlendFactor::DstColour: return "DstColour";
            case BlendFactor::OneMinusDstColour: return "OneMinusDstColour";
            case BlendFactor::SrcAlpha: return "SrcAlpha";
            case BlendFactor::OneMinusSrcAlpha: return "OneMinusSrcAlpha";
            case BlendFactor::DstAlpha: return "DstAplha";
            case BlendFactor::OneMinusDstAlpha: return "OneMinusDstAlpha";
            case BlendFactor::ConstantColour: return "ConstantColour";
            case BlendFactor::OneMinusConstantColour: return "OneMinusConstantColour";
            case BlendFactor::ConstantAlpha: return "ConstantAlpha";
            case BlendFactor::OneMinusConstantAlpha: return "OneMinusConstantAlpha";
            case BlendFactor::SrcAplhaSaturate: return "SrcAplhaSaturate";
            case BlendFactor::Src1Colour: return "Src1Colour";
            case BlendFactor::OneMinusSrc1Colour: return "OneMinusSrc1Colour";
            case BlendFactor::Src1Alpha: return "Src1Alpha";
            case BlendFactor::OneMinusSrc1Alpha: return "OneMinusSrc1Alpha";
            default:
                break;
            }
            return std::string();
        }

        std::string BlendOpToString(BlendOp op)
        {
            switch (op)
            {
            case BlendOp::Add: return "Add";
            case BlendOp::Subtract: return "Subtract";
            case BlendOp::ReverseSubtract: return "ReverseSubtract";
            case BlendOp::Min: return "Min";
            case BlendOp::Max: return "Max";
            default:
                break;
            }
            return std::string();
        }

        std::string ImageUsageFlagsToString(ImageUsageFlags flags)
        {
            std::string str;
            if (flags & ImageUsageFlagsBits::TransferSrc) { str += "TransferSrc |"; }
            if (flags & ImageUsageFlagsBits::TransferDst) { str += "TransferDst |"; }
            if (flags & ImageUsageFlagsBits::Sampled) { str += "Sampled |"; }
            if (flags & ImageUsageFlagsBits::Storage) { str += "Storage |"; }
            if (flags & ImageUsageFlagsBits::ColourAttachment) { str += "ColourAttachment |"; }
            if (flags & ImageUsageFlagsBits::DepthStencilAttachment) { str += "DepthStencilAttachment |"; }
            if (flags & ImageUsageFlagsBits::TransientAttachment) { str += "TransientAttachment |"; }
            if (flags & ImageUsageFlagsBits::InputAttachment) { str += "InputAttachment |"; }
            if (!str.empty())
            {
                str.pop_back();
            }
            return str;
        }
    }
}