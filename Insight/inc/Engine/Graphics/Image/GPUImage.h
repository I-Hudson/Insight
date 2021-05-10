#pragma once

#include "Engine/Graphics/GPUResource.h"
#include "Engine/Graphics/Enums.h"
#include "GPUImageDesc.h"
#include "Engine/Utils/Hasher.h"

namespace Insight::Graphics
{
	class GPUImage;

	/// <summary>
	/// Define a single GPUImageView. Use this to "look at" a GPUImage.
	/// </summary>
	class IS_API GPUImageView : public GPUResource
	{
	public:
		static GPUImageView* New();

		GPUImageView();
		virtual ~GPUImageView();

		void Init(GPUImage* image);

		GPUImage* GetImage() const { return m_image; }

		// [GPUResource]
		virtual ResourceType GetResourceType() const override { return ResourceType::ImageView; }
		virtual ObjectType GetObjectType() const override { return ObjectType::Other; }

	protected:
		virtual bool OnInit() = 0;

	protected:
		GPUImage* m_image;
	};

	/// <summary>
	/// Define a single GPUImage. This image can store the data when rendering to it or as a texture.
	/// </summary>
	class IS_API GPUImage : public GPUResource
	{
	public:
		GPUImage();
		virtual ~GPUImage();

		static GPUImage* New();

		void Init(GPUImageDesc desc);

		const GPUImageDesc& GetDesc() const { return m_desc; }
		GPUImageView* GetView() const { return m_view; }

		void SetLayout(const ImageLayout& newLayout);
		const bool IsTransient() const { return m_desc.IsTransient(); }

		// [GPUResource]
		virtual ResourceType GetResourceType() const override { return ResourceType::Image; }
		virtual ObjectType GetObjectType() const override { return ObjectType::Other; }

	protected:
		virtual bool OnInit() = 0;

	protected:
		GPUImageView* m_view;
		GPUImageDesc m_desc;
		ImageLayout m_imageLayout;

	private:
		friend GPUImageView;
	};

	struct GPUSamplerDesc
	{
		SamplerFilter MagFilter = SamplerFilter::Linear;
		SamplerFilter MinFilter = SamplerFilter::Linear;
		SamplerMipmapMode MipmapMode = SamplerMipmapMode::Linear;
		SamplerAddressMode AddressModeU = SamplerAddressMode::Mirrored_Repeat;
		SamplerAddressMode AddressModeV = SamplerAddressMode::Mirrored_Repeat;
		SamplerAddressMode AddressModeW = SamplerAddressMode::Mirrored_Repeat;
		float MipLodBias = 0.0f;
		CompareOp CompareOP = CompareOp::Never;
		float MinLod = 0.0f;
		float MaxLoad = 0.0f;
		float MaxAnisotropy = 1.0f;
		bool AnisortopyEnable = false;
		BorderColor BorderColor = BorderColor::Float_Opaque_White;

		u64 Hash()
		{
			Utils::Hasher hasher;
			hasher.Hash(MagFilter);
			hasher.Hash(MinFilter);
			hasher.Hash(MipmapMode);
			hasher.Hash(AddressModeU);
			hasher.Hash(AddressModeV);
			hasher.Hash(AddressModeW);
			hasher.Hash(MipLodBias);
			hasher.Hash(CompareOP);
			hasher.Hash(MinLod);
			hasher.Hash(MaxLoad);
			hasher.Hash(MaxAnisotropy);
			hasher.Hash(AnisortopyEnable);
			hasher.Hash(BorderColor);
			return hasher.GetHash();
		}
	};

	/// <summary>
	/// GPU SAMPLER
	/// </summary>
	class GPUSampler : public GPUResource
	{
	public:
		GPUSampler();
		virtual ~GPUSampler();

		static GPUSampler* New();
		static GPUSampler* TryFromCache(GPUSamplerDesc& desc);

		virtual void Init(GPUSamplerDesc& desc) = 0;
		const GPUSamplerDesc& GetDesc() const { return m_desc; }

		// [GPUResource]
		virtual ResourceType GetResourceType() const override { return ResourceType::Sampler; }
		virtual ObjectType GetObjectType() const override { return ObjectType::Other; }

	protected:
		GPUSamplerDesc m_desc;
	};
}
