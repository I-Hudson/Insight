#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/DX12/RHI_Shader_DX12.h"
#include "Graphics/RHI/DX12/RenderContext_DX12.h"

#include "Graphics/PixelFormatExtensions.h"
#include "Graphics/RHI/DX12/DX12Utils.h"

#include "FileSystem/FileSystem.h"

#include "dxcapi.h"

namespace Insight
{
    namespace Graphics
    {
        namespace RHI::DX12
        {
            IDxcBlob* RHI_Shader_DX12::GetStage(ShaderStageFlagBits stage)
            {
                int index = BitFlagsToIndex(stage);
                return m_modules[index];
            }

            void RHI_Shader_DX12::Create(RenderContext* context, ShaderDesc desc)
            {
                m_context = static_cast<RenderContext_DX12*>(context);
                m_shaderDesc = desc;

                ASSERT(m_shaderDesc.IsValid());
                const std::vector<Byte> shaderData = FileSystem::ReadFromFile(desc.ShaderDataPath, FileType::Text);
                ASSERT(!shaderData.empty());

                if (m_shaderDesc.Stages & ShaderStageFlagBits::ShaderStage_Vertex) { CompileStage(ShaderStageFlagBits::ShaderStage_Vertex, shaderData, 0); }
                if (m_shaderDesc.Stages & ShaderStageFlagBits::ShaderStage_TessControl) { CompileStage(ShaderStageFlagBits::ShaderStage_TessControl, shaderData, 1); }
                if (m_shaderDesc.Stages & ShaderStageFlagBits::ShaderStage_TessEval) { CompileStage(ShaderStageFlagBits::ShaderStage_TessEval, shaderData, 2); }
                if (m_shaderDesc.Stages & ShaderStageFlagBits::ShaderStage_Geometry) { CompileStage(ShaderStageFlagBits::ShaderStage_Geometry, shaderData, 3); }
                if (m_shaderDesc.Stages & ShaderStageFlagBits::ShaderStage_Pixel) { CompileStage(ShaderStageFlagBits::ShaderStage_Pixel, shaderData, 4); }
                if (m_shaderDesc.Stages & ShaderStageFlagBits::ShaderStage_Compute) { CompileStage(ShaderStageFlagBits::ShaderStage_Compute, shaderData, 5); }

#ifdef DX12_GROUP_SAMPLER_DESCRIPTORS
                std::vector<DescriptorBinding> samplerBindings;
                // Move all samples into a different set.
                for (auto& set : m_descriptor_sets)
                {
                    auto samplerBindingsIters = Algorithm::VectorFindAllIf(set.Bindings, [](DescriptorBinding const& binding)
                        {
                            return binding.Type == DescriptorType::Sampler;
                        });

                    for (size_t samplerIdx = 0; samplerIdx < samplerBindingsIters.size(); ++samplerIdx)
                    {
                        samplerBindings.push_back(*samplerBindingsIters.at(samplerIdx));
                        Algorithm::VectorRemove(set.Bindings, samplerBindingsIters.at(samplerIdx));
                    }
                }

                DescriptorSet samplerSet = { };
                if (m_descriptor_sets.size() > 0)
                {
                    samplerSet.Set = m_descriptor_sets.back().Set + 1;
            }
                samplerSet.Bindings = std::move(samplerBindings);
                m_descriptor_sets.push_back(samplerSet);
                samplerSet.SetHashs();
#endif

                CreateVertexInputLayout(m_shaderDesc);
        }

            void RHI_Shader_DX12::Destroy()
            {
                for (IDxcBlob*& mod : m_modules)
                {
                    if (mod)
                    {
                        mod->Release();
                        mod = nullptr;
                    }
                }
            }

            void RHI_Shader_DX12::CompileStage(ShaderStageFlagBits stage, std::string_view path, int moduleIndex)
            {
                ShaderCompiler compiler;
                IDxcBlob* code = compiler.Compile(stage, path, ShaderCompilerLanguage::Hlsl);
                compiler.GetDescriptorSets(stage, m_descriptor_sets, m_push_constant);
                m_modules[moduleIndex] = code;
            }

            void RHI_Shader_DX12::CompileStage(ShaderStageFlagBits stage, const std::vector<Byte>& shaderData, int moduleIndex)
            {
                ShaderCompiler compiler;
                IDxcBlob* code = compiler.Compile(stage, m_shaderDesc.ShaderName, shaderData, ShaderCompilerLanguage::Hlsl);
                compiler.GetDescriptorSets(stage, m_descriptor_sets, m_push_constant);
                m_modules[moduleIndex] = code;
            }

            void RHI_Shader_DX12::CreateVertexInputLayout(const ShaderDesc& desc)
            {
                if (desc.Stages & ShaderStageFlagBits::ShaderStage_Compute)
                {
                    return;
                }

                if (!desc.InputLayout.empty())
                {
                    m_shaderInputLayout = desc.InputLayout;
                }
                else
                {
                    const std::vector<Byte> shaderData = FileSystem::ReadFromFile(desc.ShaderDataPath);
                    ASSERT(!shaderData.empty());

                    ShaderCompiler compiler;
                    compiler.Compile(ShaderStage_Vertex, desc.ShaderName, shaderData, ShaderCompilerLanguage::Hlsl);
                    m_shaderInputLayout = compiler.GetInputLayout();
                }

                m_inputElements = {};
                int stride = 0;
                int previousSlot = 0;

                for (size_t i = 0; i < m_shaderInputLayout.size(); ++i)
                {
                    ShaderInputLayout& inputLayout = m_shaderInputLayout.at(i);
                    std::string semanticIndex;
                    while (inputLayout.Name.size() > 0
                        && inputLayout.Name.back() >= '0'
                        && inputLayout.Name.back() <= '9')
                    {
                        semanticIndex += inputLayout.Name.back();
                        inputLayout.Name.pop_back();
                    }
                    if (semanticIndex.size() > 0)
                    {
                        std::reverse(semanticIndex.begin(), semanticIndex.end());
                    }

                    if (previousSlot != inputLayout.Slot)
                    {
                        previousSlot = inputLayout.Slot;
                        stride = 0;
                    }

                    D3D12_INPUT_ELEMENT_DESC input;
                    input.SemanticName = inputLayout.Name.c_str();
                    input.SemanticIndex = semanticIndex.size() > 0 ? std::stoi(semanticIndex) : 0;
                    input.Format = PixelFormatToDX12(inputLayout.Format);
                    input.InputSlot = inputLayout.Slot;
                    input.AlignedByteOffset = stride;
                    input.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
                    input.InstanceDataStepRate = 0;

                    stride += PixelFormatExtensions::SizeInBytes(inputLayout.Format);
                    m_inputElements.push_back(input);
                }

                m_inputLayout.pInputElementDescs = m_inputElements.data();
                m_inputLayout.NumElements = static_cast<UINT>(m_inputElements.size());
            }
    }
}
}

#endif /// #if defined(IS_DX12_ENABLED)