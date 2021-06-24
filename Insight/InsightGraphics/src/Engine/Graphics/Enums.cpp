
#include "Engine/Graphics/Enums.h"

std::string PipelineStageFlagsToString(const PipelineStageFlags& flags)
{
    const auto addSpacer = [](std::string& str)
    {
        if (!str.empty())
        {
            str += " | ";
        }
    };

    std::string res;
    if (flags & (u32)PipelineStage::Top_Of_Pipe) { res += "Top Of Pipe"; }
    else if (flags & (u32)PipelineStage::Vertex_Input) { addSpacer(res); res += "Vertex Input"; }
    else if (flags & (u32)PipelineStage::Vertex_Shader) { addSpacer(res); res += "Vertex Shader"; }
    else if (flags & (u32)PipelineStage::Tess_Control_Shader) { addSpacer(res); res += "Tess Control Shader"; }
    else if (flags & (u32)PipelineStage::Tess_Evaluation_Shader) { addSpacer(res); res += "Tess Evaluation Shader"; }
    else if (flags & (u32)PipelineStage::Geometry_Shader) { addSpacer(res); res += "Geometry Shader"; }
    else if (flags & (u32)PipelineStage::Fragment_Shader) { addSpacer(res); res += "Fragment Shader"; }
    else if (flags & (u32)PipelineStage::Early_Fragment_Test) { addSpacer(res); res += "Early Fragment Test"; }
    else if (flags & (u32)PipelineStage::Late_Fragment_Test) { addSpacer(res); res += "Late Fragment Test"; }
    else if (flags & (u32)PipelineStage::Color_Attachment_Output) { addSpacer(res); res += "Color Attachment Output"; }
    else if (flags & (u32)PipelineStage::Compute_Shader) { addSpacer(res); res += "Compute Shader"; }
    else if (flags & (u32)PipelineStage::Transfer) { addSpacer(res); res += "Transfer"; }
    else if (flags & (u32)PipelineStage::Bottom_Of_Pipe) { addSpacer(res); res += "Bottom Of Pipe"; }
    else if (flags & (u32)PipelineStage::Host) { addSpacer(res); res += "Host"; }
    else if (flags & (u32)PipelineStage::All_Graphics) { addSpacer(res); res += "All Graphics"; }
    else if (flags & (u32)PipelineStage::All_Commands) { addSpacer(res); res += "All Commands"; }

    return res;
}

const char* ImageLayoutToString(const ImageLayout& layout)
{
    switch (layout)
    {
    case ImageLayout::Undefined:                return "Undefined";
    case ImageLayout::General:                  return "General";
    case ImageLayout::Color_Attachment:         return "Color Attachment";
    case ImageLayout::Depth_Stencil_Attachment: return "Depth Stencil Attachment";
    case ImageLayout::Depth_Stencil_Read_Only:  return "Depth Stencil Read Only";
    case ImageLayout::Shader_Read_Only:         return "Shader Read Only";
    case ImageLayout::Transfer_Src:             return "Transfer Src";
    case ImageLayout::Transfer_Dst:             return "Transfer Dst";
    case ImageLayout::Preinitialized:           return "Preinitialized";
    }
    return "";
}

std::string AccessFlagsToString(const AccessFlags& flags)
{
    const auto addSpacer = [](std::string& str)
    {
        if (!str.empty())
        {
            str += " | ";
        }
    };

    std::string res;
    if (flags & (u32)Access::Indirect_Command_Read) { res += "Indirect Command Read"; }
    else if (flags & (u32)Access::Index_Read) { addSpacer(res); res += "Index Read"; }
    else if (flags & (u32)Access::Vertex_Attribute_Read) { addSpacer(res); res += "Vertex Attribute Read"; }
    else if (flags & (u32)Access::Uniform_Read) { addSpacer(res); res += "Uniform Read"; }
    else if (flags & (u32)Access::Input_Attachmnet_Read) { addSpacer(res); res += "Input Attachmnet Read"; }
    else if (flags & (u32)Access::Shader_Read) { addSpacer(res); res += "Shader Read"; }
    else if (flags & (u32)Access::Shader_Write) { addSpacer(res); res += "Shader_Write"; }
    else if (flags & (u32)Access::Color_Attachment_Read) { addSpacer(res); res += "Color Attachment Read"; }
    else if (flags & (u32)Access::Color_Attachment_Write) { addSpacer(res); res += "Color Attachment Write"; }
    else if (flags & (u32)Access::Depth_Stencil_Attachment_Read) { addSpacer(res); res += "Depth Stencil Attachment Read"; }
    else if (flags & (u32)Access::Depth_Stencil_Attachment_Write) { addSpacer(res); res += "Depth Stencil Attachment Write"; }
    else if (flags & (u32)Access::Transfer_Read) { addSpacer(res); res += "Transfer Read"; }
    else if (flags & (u32)Access::Transfer_Write) { addSpacer(res); res += "Transfer Write"; }
    else if (flags & (u32)Access::Host_Read) { addSpacer(res); res += "Host Read"; }
    else if (flags & (u32)Access::Host_Write) { addSpacer(res); res += "Host Write"; }
    else if (flags & (u32)Access::Memory_Read) { addSpacer(res); res += "Memory Read"; }
    else if (flags & (u32)Access::Memory_Write) { addSpacer(res); res += "Memory Write"; }

    return res;
}