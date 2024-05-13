#pragma once

#include "Graphics/Defines.h"
#include "Graphics/Enums.h"
#include "Graphics/PixelFormat.h"

#include <unordered_map>
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

		struct IS_GRAPHICS AttachmentDescription
		{
			AttachmentDescription();

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

			bool IsValid() const
			{
				return FinalLayout != ImageLayout::Undefined;
			}

			u64 GetHash() const;

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
			u64 GetHash() const;

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