#pragma once

#include "Graphics/Enums.h"
#include "Graphics/ShaderDesc.h"

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

		/// Pipeline state object struct. Store all current information about the render pass.
		struct PipelineStateObject
		{
			static const int RenderTargetCount = 8;

			std::string Name;
			ShaderDesc ShaderDescription;
			GPUQueue Queue = GPUQueue_Graphics;

			std::array<RHI_Texture*, RenderTargetCount> RenderTargets;
			RHI_Texture* DepthStencil = nullptr;

			PrimitiveTopologyType PrimitiveTopologyType = PrimitiveTopologyType::TriangleList;
			PolygonMode PolygonMode;
			CullMode CullMode;
			FrontFace FrontFace;

			bool DepthTest = true;
			bool DepthWrite = true;
			CompareOp DepthCompareOp = CompareOp::LessOrEqual;
			bool DepthBaisEnabled = false;
			bool DepthClampEnabled = false;

			float DepthConstantBaisValue = 0.0f;
			float DepthSlopeBaisValue = 0.0f;

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