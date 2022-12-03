#pragma once

#include "Graphics/Enums.h"
#include "Graphics/PixelFormat.h"

#include <glm/ext/vector_float4.hpp>
#include <glm/ext/vector_float2.hpp>

#include <unordered_map>
#include <array>
#include <vector>

namespace Insight
{
	namespace Graphics
	{
		class RenderContext;
		class RHI_Texture;
		class RHI_RenderpassManager;
		class RenderGraph;
		struct PipelineStateObject;

		struct AttachmentDescription
		{
			PixelFormat Format;
			AttachmentLoadOp LoadOp = AttachmentLoadOp::Clear;
			AttachmentStoreOp StoreOp = AttachmentStoreOp::Store;
			AttachmentLoadOp StencilLoadOp = AttachmentLoadOp::DontCare;
			AttachmentStoreOp StencilStoreOp = AttachmentStoreOp::DontCare;
			ImageLayout InitalLayout = ImageLayout::Undefined;
			ImageLayout FinalLayout = ImageLayout::Undefined;

			///===================================================================
			/// Used for rendering 
			u32 Layer_Array_Index = 0;
			glm::vec4 ClearColour = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
			glm::vec2 DepthStencilClear = glm::vec2(1.0f, 0.0f);

			bool IsValid() const
			{
				return FinalLayout != ImageLayout::Undefined;
			}

			u64 GetHash() const
			{
				u64 hash = 0;
			
				HashCombine(hash, Format);
				HashCombine(hash, LoadOp);
				HashCombine(hash, StoreOp);
				HashCombine(hash, StencilLoadOp);
				HashCombine(hash, StencilStoreOp);
				HashCombine(hash, InitalLayout);
				HashCombine(hash, FinalLayout);

				return hash;
			}

			static AttachmentDescription Default(PixelFormat format, ImageLayout finalLayout)
			{
				AttachmentDescription desc = { };
				desc.Format = format;
				desc.FinalLayout = finalLayout;
				return desc;
			}

			static AttachmentDescription Load(PixelFormat format, ImageLayout finalLayout)
			{
				AttachmentDescription description = { };
				description.LoadOp = AttachmentLoadOp::Load;
				description.Format = format;
				description.FinalLayout = finalLayout;
				return description;
			}

			static AttachmentDescription DontCare(PixelFormat format, ImageLayout finalLayout)
			{
				AttachmentDescription description = { };
				description.LoadOp = AttachmentLoadOp::DontCare;
				description.Format = format;
				description.FinalLayout = finalLayout;
				return description;
			}
		};

		struct RenderpassDescription
		{
			std::vector<RHI_Texture*> ColourAttachments;
			std::vector<AttachmentDescription> Attachments;

			RHI_Texture* DepthStencil = nullptr;
			AttachmentDescription DepthStencilAttachment;

			bool AllowDynamicRendering = true;

			void AddAttachment(AttachmentDescription description)
			{
				Attachments.push_back(description);
			}

			u64 GetHash() const
			{
				u64 hash = 0;
				for (const auto& texture : ColourAttachments)
				{
					HashCombine(hash, texture);
				}
				for (const auto& attachment : Attachments)
				{
					HashCombine(hash, attachment.GetHash());
				}

				if (DepthStencil)
				{
					HashCombine(hash, DepthStencil);
					HashCombine(hash, DepthStencilAttachment.GetHash());
				}

				HashCombine(hash, SwapchainPass);

				return hash;
			}

			/// [PRIVATE] This should not be set manually by the user. RenderContext's and other graphics
			/// API use this.
			bool SwapchainPass = false;
			PipelineStateObject* Pso = nullptr;
		private:

			friend class RHI_RenderpassManager;
			friend class RenderGraph;
		};

		struct RHI_Renderpass
		{
			void* Resource;
		};

		class RHI_RenderpassManager
		{
		public:

			void SetRenderContext(RenderContext* context);

			RHI_Renderpass GetOrCreateRenderpass(RenderpassDescription& description);
			RHI_Renderpass GetRenderpass(u64 hash) const;

			void Release(u64 hash, bool remove = true);
			void Release(RenderpassDescription description, bool remove = true);
			void ReleaseAll();

		private:
			void PrepreRenderpass(RenderpassDescription& description);
			RHI_Renderpass AddRenderpass(RenderpassDescription& description);

		private:
			RenderContext* m_context = nullptr;
			std::unordered_map<u64, RHI_Renderpass> m_renderpasses;
		};
	}
}