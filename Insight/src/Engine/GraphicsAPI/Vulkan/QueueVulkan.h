#pragma once
#include "VulkanHeaders.h"
#include "Engine/Core/InsightAlias.h"
#include "Engine/Core/Compiler.h"
#include "Engine/Platform/CriticalSection.h"

class GPUDeviceVulkan;
class CmdBufferVulkan;

class QueueVulkan
{
public:
	QueueVulkan(GPUDeviceVulkan* device, U32 familyIndex);

	INLINE U32 GetFamilyIndex() const { return m_familyIndex; }

	void Submit(CmdBufferVulkan* cmdBuffer, U32 numSignalSemaphores = 0, VkSemaphore* signalSemaphores = nullptr);
	INLINE void Submit(CmdBufferVulkan* cmdBuffer, VkSemaphore signalSemaphore)
	{
		Submit(cmdBuffer, 1, &signalSemaphore);
	}

	INLINE VkQueue GetHandle() const { return m_queue; }
	
	void GetLastSubmittedInfo(CmdBufferVulkan*& cmdBuffer, U64& fenceCounter) const;

	INLINE U64 GetSubmnitCount() const { return m_submitCounter; }

private:
	void UpdateLastSubmittedCommandBuffer(CmdBufferVulkan* cmdBuffer);

private:
	VkQueue m_queue;
	U32 m_familyIndex;
	U32 m_queueIndex;
	GPUDeviceVulkan* m_device;
	CriticalSection m_locker;
	CmdBufferVulkan* m_lastSubmittedCmdBuffer;
	U64 m_lastSubmittedCmdBufferFenceCounter;
	U64 m_submitCounter;
};

