#pragma once

#include "Engine/Graphics/GPUResource.h"
#include "Engine/Graphics/Enums.h"
#include "GPUImageDescription.h"

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
/// Define a single GPUImage. This image stores the data when rendering to it.
/// </summary>
class IS_API GPUImage : public GPUResource
{
public:
	static GPUImage* New();

	GPUImage();
	virtual ~GPUImage();

	void Init(GPUImageDescription desc);

	const GPUImageDescription& GetDesc() const { return m_desc; }
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
	GPUImageDescription m_desc;
	ImageLayout m_imageLayout;

private:
	friend GPUImageView;
};

