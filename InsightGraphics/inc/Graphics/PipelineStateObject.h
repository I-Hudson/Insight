#pragma once

#include "Graphics/Enums.h"
#include <vector>

namespace Insight
{
	namespace Graphics
	{
		class RHI_Shader;
		class RenderTarget;

		// Pipeline state object struct. Store all current information about the render pass.
		struct PipelineStateObject
		{
			std::wstring Name;
			RHI_Shader* Shader = nullptr;
			GPUQueue Queue = GPUQueue_Graphics;

			std::vector<RenderTarget*> RenderTargets;

			PrimitiveTopologyType PrimitiveTopologyType = PrimitiveTopologyType::TriangleList;
			PolygonMode PolygonMode;
			CullMode CullMode;
			FrontFace FrontFace;

			bool DepthTest = true;
			bool DepthWrite = true;
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

			bool Swapchain = false;

			u64 GetHash() const
			{
				u64 hash = 0;

				HashCombine(hash, Shader);
				HashCombine(hash, Queue);

				for (const RenderTarget* rt : RenderTargets)
				{
					HashCombine(hash, rt);
				}

				HashCombine(hash, PrimitiveTopologyType);
				HashCombine(hash, PolygonMode);
				HashCombine(hash, CullMode);
				HashCombine(hash, FrontFace);

				HashCombine(hash, DepthTest);
				HashCombine(hash, DepthWrite);
				HashCombine(hash, DepthBaisEnabled);
				HashCombine(hash, DepthClampEnabled);

				HashCombine(hash, BlendEnable);
				HashCombine(hash, ColourWriteMask);
				HashCombine(hash, SrcColourBlendFactor);
				HashCombine(hash, DstColourBlendFactor);
				HashCombine(hash, ColourBlendOp);
				HashCombine(hash, SrcAplhaBlendFactor);
				HashCombine(hash, DstAplhaBlendFactor);
				HashCombine(hash, AplhaBlendOp);

				HashCombine(hash, Swapchain);

				return hash;
			}

			bool IsValid() const
			{
				return Shader;
			}
		};
	}
}