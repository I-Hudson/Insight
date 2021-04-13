#pragma once

#include "Engine/Graphics/GPUResource.h"
#include "Engine/Graphics/Enums.h"
#include "GPUImageDesc.h"

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

}
