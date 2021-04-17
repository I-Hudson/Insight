#include "ispch.h"
#include "Engine/GraphicsAPI/Vulkan/GPUSyncVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/GPUDeviceVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanInitializers.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanUtils.h"

namespace Insight::GraphicsAPI::Vulkan
{
	GPUFenceVulkan::GPUFenceVulkan()
	{ }

	GPUFenceVulkan::~GPUFenceVulkan()
	{
		ReleaseGPU();
	}

	void GPUFenceVulkan::Init(const Graphics::GPUFenceDesc& desc)
	{
		m_desc = desc;
		m_desc.FenceManager = GPUDevice::Instance()->GetDefaultFenceManager();
		ASSERT(m_desc.FenceManager != nullptr && "[GPUFenceVulkan::Init] 'FenceManager' must be a valid pointer.");

		VkFenceCreateInfo info = vks::initializers::fenceCreateInfo(m_desc.IsSignaled);
		ThrowIfFailed(vkCreateFence(m_device->Device, &info, nullptr, &m_fenceVulkan));
		m_memoryUsage = 1;

		CastManager()->AddFence(this);	
	}

	void GPUFenceVulkan::Wait()
	{
		vkWaitForFences(m_device->Device, 1, &m_fenceVulkan, VK_TRUE, U64_MAX);
	}

	void GPUFenceVulkan::Reset()
	{
		vkResetFences(m_device->Device, 1, &m_fenceVulkan);
	}

	VkFence* GPUFenceVulkan::GetHandleVulkan()
	{
		return &m_fenceVulkan;
	}

	void GPUFenceVulkan::SetName(const std::string& name)
	{
		m_name = name;
		if (GPUDebugMarkerVulkan::IsInitialised())
		{
			GPUDebugMarkerVulkan::Instance()->SetObjectName(m_name, Graphics::Debug::DebugObject::Fence, (u64)m_fenceVulkan);
		}
	}

	void GPUFenceVulkan::OnReleaseGPU()
	{
		vkDestroyFence(m_device->Device, m_fenceVulkan, nullptr);
		CastManager()->ReleaseFence(this);
	}

	GPUFenceManagerVulkan* GPUFenceVulkan::CastManager()
	{
		return static_cast<GPUFenceManagerVulkan*>(m_desc.FenceManager);
	}


	/// <summary>
	/// GPUFenceManagerVulkan
	/// </summary>
	GPUFenceManagerVulkan::GPUFenceManagerVulkan()
	{ }

	GPUFenceManagerVulkan::~GPUFenceManagerVulkan()
	{
		Release();
	}

	void GPUFenceManagerVulkan::Release()
	{
		for (auto* fence : m_usedList)
		{
			fence->ReleaseGPU();
			::Delete(fence);
		}
	}

	void GPUFenceManagerVulkan::AddFence(Graphics::GPUFence* fence)
	{
		m_usedList.push_back(fence);
	}

	Graphics::GPUFence* GPUFenceManagerVulkan::QueryFreeFence()
	{
		if (m_freeList.size() > 0)
		{
			Graphics::GPUFence* freeFence = m_freeList.back();
			m_freeList.pop_back();
			m_usedList.push_back(freeFence);
			return freeFence;
		}
		return nullptr;
	}

	void GPUFenceManagerVulkan::ReleaseFence(const Graphics::GPUFence* fence)
	{
		ASSERT(std::find(m_usedList.begin(), m_usedList.end(), fence) != m_usedList.end() && 
			   "[GPUFenceManagerVulkan::ReleaseFence] Fence is not in use. Cannot be released.");
		m_usedList.erase(std::find(m_usedList.begin(), m_usedList.end(), fence));
	}


	/// <summary>
	/// GPUSemaphoreVulkan
	/// </summary>
	GPUSemaphoreVulkan::GPUSemaphoreVulkan()
	{ }

	GPUSemaphoreVulkan::~GPUSemaphoreVulkan()
	{
		ReleaseGPU();
	}

	void GPUSemaphoreVulkan::Init(const Graphics::GPUSemaphoreDesc& desc)
	{
		m_desc = desc;
		m_desc.SemaphoreManager = GPUDevice::Instance()->GetDefaultSignalManager();
		ASSERT(m_desc.SemaphoreManager != nullptr && "[GPUSemaphoreVulkan::Init] 'SemaphoreManager' must be a valid pointer.");

		VkSemaphoreCreateInfo info = vks::initializers::semaphoreCreateInfo();
		vkCreateSemaphore(m_device->Device, &info, nullptr, &m_semaphoreVulkan);
		m_memoryUsage = 1;

		CastManager()->AddSemaphore(this);
	}

	void GPUSemaphoreVulkan::Signal()
	{
		VkSemaphoreSignalInfo info = vks::initializers::semaphoreSignalInfo();
		info.semaphore = m_semaphoreVulkan;
		vkSignalSemaphore(m_device->Device, &info);
	}

	void GPUSemaphoreVulkan::Wait()
	{
		VkSemaphoreWaitInfo info = vks::initializers::semaphoreWaitInfo();
		info.semaphoreCount = 1;
		info.pSemaphores = &m_semaphoreVulkan;
		vkWaitSemaphores(m_device->Device, &info, U64_MAX);
	}

	void GPUSemaphoreVulkan::SetName(const std::string& name)
	{
		m_name = name;
		if (GPUDebugMarkerVulkan::IsInitialised())
		{
			GPUDebugMarkerVulkan::Instance()->SetObjectName(m_name, Graphics::Debug::DebugObject::Seamaphone, (u64)m_semaphoreVulkan);
		}
	}

	void GPUSemaphoreVulkan::OnReleaseGPU()
	{
		vkDestroySemaphore(m_device->Device, m_semaphoreVulkan, nullptr);
		CastManager()->ReleaseSemaphore(this);
	}

	GPUSemaphoreManagerVulkan* GPUSemaphoreVulkan::CastManager()
	{
		return static_cast<GPUSemaphoreManagerVulkan*>(m_desc.SemaphoreManager);
	}


	/// <summary>
	/// GPUSemaphoreManagerVulkan
	/// </summary>
	GPUSemaphoreManagerVulkan::GPUSemaphoreManagerVulkan()
	{ }

	GPUSemaphoreManagerVulkan::~GPUSemaphoreManagerVulkan()
	{
		ASSERT(m_semaphores.size() == 0 && "[GPUSemaphoreManagerVulkan::~GPUSemaphoreManagerVulkan] Not all semaphores have been released.");
	}

	void GPUSemaphoreManagerVulkan::AddSemaphore(Graphics::GPUSemaphore* fence)
	{
		ASSERT(std::find(m_semaphores.begin(), m_semaphores.end(), fence) == m_semaphores.end() &&
			   "[GPUFenceManagerVulkan::AddSemaphore] Semaphore has already been added.");
		m_semaphores.push_back(fence);
	}

	void GPUSemaphoreManagerVulkan::ReleaseSemaphore(const Graphics::GPUSemaphore* fence)
	{
		ASSERT(std::find(m_semaphores.begin(), m_semaphores.end(), fence) != m_semaphores.end() &&
			   "[GPUFenceManagerVulkan::ReleaseFence] Fence is not in use. Cannot be released.");
		m_semaphores.erase(std::find(m_semaphores.begin(), m_semaphores.end(), fence));
	}
}