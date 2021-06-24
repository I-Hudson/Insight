#pragma once
#include "Engine/Graphics/GPUResource.h"

namespace Insight::Graphics
{
	class GPUFenceManager;
	class GPUSemaphoreManager;

	struct GPUFenceDesc
	{
		GPUFenceDesc(bool isSignaled = false)
			: IsSignaled(isSignaled)
		{ }
		GPUFenceManager* FenceManager;
		bool IsSignaled;
	};

	class GPUFence : public GPUResource
	{
	public:
		GPUFence() : m_desc(GPUFenceDesc()) { }
		virtual ~GPUFence() { }

		static GPUFence* New();

		virtual void Init(const GPUFenceDesc& desc) = 0;
		virtual void Wait() = 0;
		virtual void Reset() = 0;
		const bool& IsSignaled() const { return m_desc.IsSignaled; }

		//[GPUResource]
		virtual ResourceType GetResourceType() const override { return ResourceType::Fence; }
		virtual ObjectType GetObjectType() const override { return ObjectType::Other; }

	protected:
		GPUFenceDesc m_desc;

	private:
		friend class GPUFenceManager;
	};

	class GPUFenceManager
	{
	public:
		virtual ~GPUFenceManager() { }

		virtual void Release() = 0;

	protected:
		virtual void AddFence(GPUFence* fence) = 0;
		virtual GPUFence* QueryFreeFence() = 0;
		virtual void ReleaseFence(const GPUFence* fence) = 0;
	};


	struct GPUSemaphoreDesc
	{
		GPUSemaphoreDesc(bool isSignaled = false)
			: IsSignaled(isSignaled)
		{ }

		GPUSemaphoreManager* SemaphoreManager;
		bool IsSignaled;
	};

	class GPUSemaphore : public GPUResource
	{
	public:
		GPUSemaphore() : m_desc(GPUSemaphoreDesc()) { }
		virtual ~GPUSemaphore() { }

		static GPUSemaphore* New();

		virtual void Init(const GPUSemaphoreDesc& desc) = 0;
		virtual void Signal() = 0;
		virtual void Wait() = 0;
		const bool& IsSignaled() const { return m_desc.IsSignaled; }

		//[GPUResource]
		virtual ResourceType GetResourceType() const override { return ResourceType::Semaphore; }
		virtual ObjectType GetObjectType() const override { return ObjectType::Other; }

	protected:
		GPUSemaphoreDesc m_desc;
	};

	class GPUSemaphoreManager
	{
	public:
		virtual ~GPUSemaphoreManager() { }

	protected:
		virtual void AddSemaphore(GPUSemaphore* fence) = 0;
		virtual void ReleaseSemaphore(const GPUSemaphore* fence) = 0;
	};
}