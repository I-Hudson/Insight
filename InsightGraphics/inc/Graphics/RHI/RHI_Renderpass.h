#pragma once

#include "Graphics/Enums.h"
#include "Graphics/PixelFormat.h"

#include <unordered_map>
#include <array>
#include <vector>

namespace Insight
{
	namespace Graphics
	{
		class RenderContext;
		class RHI_Texture;

		struct AttachmentDescription
		{
			PixelFormat Format;
			AttachmentLoadOp LoadOp = AttachmentLoadOp::Clear;
			AttachmentStoreOp StoreOp = AttachmentStoreOp::Store;
			AttachmentLoadOp StencilLoadOp = AttachmentLoadOp::DontCare;
			AttachmentStoreOp StencilStoreOp = AttachmentStoreOp::DontCare;
			ImageLayout InitalLayout = ImageLayout::Undefined;
			ImageLayout FinalLayout = ImageLayout::Undefined;

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

				return hash;
			}
		};

		struct RHI_Renderpass
		{
			void* Resource;
		};

		class RHI_RenderpassManager
		{
		public:

			void SetRenderContext(RenderContext* context);

			RHI_Renderpass GetOrCreateRenderpass(RenderpassDescription description);

			void Release(u64 hash, bool remove = true);
			void Release(RenderpassDescription description, bool remove = true);
			void ReleaseAll();

		private:
			RHI_Renderpass AddRenderpass(RenderpassDescription description);

		private:
			RenderContext* m_context = nullptr;
			std::unordered_map<u64, RHI_Renderpass> m_renderpasses;
		};
	}
}