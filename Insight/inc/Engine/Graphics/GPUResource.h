#pragma once
#include "Engine/Core/Common.h"
#include "Engine/Core/NonCopyable.h"
#include "Engine/Graphics/Debug/GPUDebug.h"

class IS_API GPUResource : public Object, public NonCopyable
{
public:
	/// <summary>
	/// GPU Resources types.
	/// </summary>
	DECLARE_ENUM_16(ResourceType, RenderTarget, Image, ImageView, Sampler, Texture, CubeTexture, VolumeTexture, Buffer, Shader, PipelineState, Descriptor, Query, CommandBuffer, CommandPool, Fence, Semaphore);

	/// <summary>
	/// GPU Resources object types. Used to detect Texture objects from subset of Types:  RenderTarget, Texture, CubeTexture, VolumeTexture which use the same API object.
	/// </summary>
	DECLARE_ENUM_3(ObjectType, Texture, Buffer, Other);

	GPUResource()
	{ }

	virtual ~GPUResource()
	{
#ifdef IS_DEBUG
		IS_CORE_ASSERT(m_memoryUsage == 0, "GPUResource has not been released.");
#endif
	}

	virtual ResourceType GetResourceType() const = 0;
	virtual ObjectType GetObjectType() const = 0;

	virtual void OnDeviceDispose() 
	{
		ReleaseGPU();
	}

	FORCE_INLINE U64 GetMemoryUsage() const { return m_memoryUsage; }

	void ReleaseGPU()
	{
		if (m_memoryUsage != 0)
		{
			OnReleaseGPU();
			m_memoryUsage = 0;
		}
	};

	virtual void SetName(const std::string& name) = 0;
	const std::string& GetName() const { return m_name; }

protected:
	virtual void OnReleaseGPU() = 0;
	U64 m_memoryUsage = 0;

	std::string m_name;
};

template<typename Device, typename Resouce>
class GPUResouceBase : public Resouce
{
public:
	GPUResouceBase()
	{
		m_device = dynamic_cast<Device*>(GPUDevice::Instance());
		m_type.SetType<Resouce>();

		// Register
		m_device->Resources.Add(this);
	}

	virtual ~GPUResouceBase()
	{
		// Unregister
		if (m_device)
		{
			m_device->Resources.Remove(this);
		}
	}

protected:
	Type m_type;
	Device* m_device;
};