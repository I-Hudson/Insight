#pragma once

#include "Graphics/Enums.h"
#include "Graphics/ShaderDesc.h"
#include "Graphics/Descriptors.h"

#include <array>
#include <memory>

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float4.hpp>

namespace Insight
{
	namespace Graphics
	{
		class RHI_Shader;
		class RHI_Texture;

		/// @brief Define of a pipeline (this is the same as a root signature in DX12.)
		struct PipelineLayout
		{
			/// @brief Define all descriptor sets (DX12 descriptor tables) for this layout (root signature).
			std::vector<DescriptorSet> Sets;
			/// @brief Define all push constants (DX12 Root descriptors) for this layout (root signature).
			std::vector<PushConstant> PushConstants;

			bool IsValid() const;
		};

		/// Pipeline state object struct. Store all current information about the render pass.
		struct PipelineStateObject
		{
			static const int RenderTargetCount = 8;

			/// @brief The layout used for the Pipeline. (If defined is used over shader reflection).
			PipelineLayout Layout;

			std::string Name;
			ShaderDesc ShaderDescription;
			GPUQueue Queue = GPUQueue_Graphics;

			std::array<RHI_Texture*, RenderTargetCount> RenderTargets;
			glm::vec4 RenderTargetClearValues[RenderTargetCount];

			RHI_Texture* DepthStencil = nullptr;
			glm::vec2 DepthSteniclClearValue = glm::vec2(1.0f, 0.0f);

			PrimitiveTopologyType PrimitiveTopologyType = PrimitiveTopologyType::TriangleList;
			PolygonMode PolygonMode;
			CullMode CullMode;
			FrontFace FrontFace;

			bool DepthTest = true;
			bool DepthWrite = true;
			CompareOp DepthCompareOp = CompareOp::LessOrEqual;
			bool DepthBaisEnabled = false;
			bool DepthClampEnabled = false;

			bool BlendEnable = false;
			ColourComponentFlags ColourWriteMask = ColourComponentR | ColourComponentG | ColourComponentB | ColourComponentA;
			BlendFactor SrcColourBlendFactor = {};
			BlendFactor DstColourBlendFactor = {};
			BlendOp ColourBlendOp = {};
			BlendFactor SrcAplhaBlendFactor = {};
			BlendFactor DstAplhaBlendFactor = {};
			BlendOp AplhaBlendOp = {};

			std::vector<DynamicState> Dynamic_States{ DynamicState::Viewport, DynamicState::Scissor };

			bool AllowDynamicRendering = true;

			/// Private members. Should not be touched.
			bool Swapchain = false;
			RHI_Shader* Shader = nullptr;
			u64 Renderpass = 0;

			u64 GetHash() const;

			bool IsValid() const
			{
				return Shader;
			}
		};
	}
}