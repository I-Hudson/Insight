#pragma once
#include "Engine/Core/Common.h"
#include "Engine/Core/NonCopyable.h"

class IS_API GPUResource : public Object, public NonCopyable
{
public:
	/// <summary>
	/// GPU Resources types.
	/// </summary>
	DECLARE_ENUM_9(ResourceType, RenderTarget, Texture, CubeTexture, VolumeTexture, Buffer, Shader, PipelineState, Descriptor, Query);

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
	}

protected:
	virtual void OnReleaseGPU() = 0;
	virtual bool CreateGPUResource() { return false; }
	U64 m_memoryUsage = 0;
};

/// <summary>
/// Describes base implementation of Graphics Device resource for rendering back-ends.
/// </summary>
/// <remarks>
/// DeviceType is GPU device typename, BaseType must be GPUResource type to inherit from).
/// </remarks>
template<class DeviceType, class BaseType>
class GPUResourceBase : public BaseType
{
public:

	/// <summary>
	/// Initializes a new instance of the <see cref="GPUResourceBase"/> class.
	/// </summary>
	/// <param name="device">The graphics device.</param>
	/// <param name="name">The resource name.</param>
	GPUResourceBase(DeviceType* device, const std::string& name) noexcept
		: m_device(device)
		, m_name(name)
	{
		ASSERT(device);

		// Register
		device->Resources.Add(this);
	}

	/// <summary>
	/// Finalizes an instance of the <see cref="GPUResourceBase"/> class.
	/// </summary>
	virtual ~GPUResourceBase()
	{
		// Unregister
		if (m_device)
			m_device->Resources.Remove(this);
	}

	/// <summary>
	/// Gets the graphics device.
	/// </summary>
	/// <returns>The device.</returns>
	FORCE_INLINE DeviceType* GetDevice() const
	{
		return m_device;
	}

	virtual void OnDeviceDispose() override
	{
		// Base
		GPUResource::OnDeviceDispose();

		// Unlink device handle
		m_device = nullptr;
	}

	const std::string& GetName() const { return m_name; }

protected:
	DeviceType* m_device;
	std::string m_name;
};